SELECT dtl.line_number                                                          LINE_NUMBER,
       TO_CHAR(
           ABS(
               CASE
                WHEN tpy.type IN ('INV', 'DM') THEN
                    dtl.quantity_invoiced
                ELSE
                    dtl.quantity_credited
               END
           ),
           'FM999999999999990D00'
       )                                                                        QUANTITY,
       'NIU'                                                                    UOM,
       dtl.description                                                          DESCRIPTION,
       TO_CHAR(ABS(dtl.extended_amount), 'FM999999999999990D00')                AMOUNT,
       CASE WHEN tax.customer_trx_id IS NOT NULL THEN 'true' ELSE 'false' END   HAS_TAX,
       TO_CHAR(NVL(tax.amount, 0), 'FM999999999999990D00')                      VAT_AMOUNT,
       TO_CHAR(NVL(tax.rate, 0), 'FM999999999999990D00')                        VAT_RATE
FROM ra_customer_trx_all        trx,
     ra_customer_trx_lines_all  dtl,
     ra_cust_trx_types_all      tpy,
     (
        SELECT *
        FROM (
                SELECT SUM(ABS(dtl.taxable_amount))                                                                                                        TAXABLE_AMOUNT,
                       SUM(ABS(dtl.extended_amount))                                                                                                       AMOUNT,
                       ABS(tax.tax_rate)                                                                                                                   RATE,
                       dtl.customer_trx_id,
                       dtl.link_to_cust_trx_line_id
                FROM ra_customer_trx_all        trx,
                     ra_cust_trx_types_all      tpy,
                     ra_customer_trx_lines_all  dtl,
                     ar_vat_tax_all_vl          tax
                WHERE trx.customer_trx_id   = dtl.customer_trx_id
                AND   trx.cust_trx_type_id  = tpy.cust_trx_type_id
                AND   dtl.vat_tax_id        = tax.vat_tax_id
                AND   dtl.line_type         = 'TAX'
                GROUP BY ABS(tax.tax_rate), tax.attribute2, dtl.link_to_cust_trx_line_id, dtl.customer_trx_id
                HAVING SUM(dtl.extended_amount) != 0
             )
        WHERE AMOUNT != 0
     )                          tax
WHERE trx.customer_trx_id       = dtl.customer_trx_id
AND   trx.cust_trx_type_id      = tpy.cust_trx_type_id
AND   dtl.customer_trx_id       = tax.customer_trx_id(+)
AND   dtl.customer_trx_line_id  = tax.link_to_cust_trx_line_id(+)
AND   dtl.line_type             = 'LINE'
AND   trx.customer_trx_id       = ?
ORDER BY dtl.line_number