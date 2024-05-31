SELECT *
FROM (
        SELECT TO_CHAR(
                SUM(ABS(dtl.taxable_amount)),
                'FM999999999999990D00'
               )                                                                                                                                   TAXABLE_AMOUNT,
               TO_CHAR(
                SUM(ABS(dtl.extended_amount)),
                'FM999999999999990D00'
               )                                                                                                                                    AMOUNT,
               TO_CHAR(ABS(tax.tax_rate), 'FM999999999999990D00')                                                                                   RATE
        FROM ra_customer_trx_all        trx,
             ra_cust_trx_types_all      tpy,
             ra_customer_trx_lines_all  dtl,
             ar_vat_tax_all_vl          tax
        WHERE trx.customer_trx_id   = dtl.customer_trx_id
        AND   trx.cust_trx_type_id  = tpy.cust_trx_type_id
        AND   dtl.vat_tax_id        = tax.vat_tax_id
        AND   dtl.line_type         = 'TAX'
        AND   trx.customer_trx_id   = ?
        GROUP BY ABS(tax.tax_rate)
        HAVING SUM(dtl.extended_amount) != 0
     )
WHERE AMOUNT != 0