SELECT TO_CHAR(ABS(SUM(dtl.tbt_extended_amount)), 'FM999999999999990D00')                                                                       TOTAL_BEFORE_TAX,
       TO_CHAR(ABS(SUM(dtl.tat_extended_amount)), 'FM999999999999990D00')                                                                       TOTAL_AFTER_TAX,
       TO_CHAR(
           NVL(
            ABS(
                SUM(
                    (
                        SELECT MAX(CASE WHEN ((dtl.type IN ('INV', 'DM') AND alt.taxable_amount > 0) OR (dtl.type = 'CM' AND alt.taxable_amount < 0)) THEN alt.taxable_amount ELSE 0 END)
                        FROM ra_customer_trx_lines_all  alt,
                             ar_vat_tax_all_vl          tax
                        WHERE alt.vat_tax_id                = tax.vat_tax_id
                        AND   alt.line_type                 = 'TAX'
                        AND   alt.tax_rate                  != 0
                        AND   tax.tax_code LIKE 'IVA%'
                        AND   alt.link_to_cust_trx_line_id 	= dtl.customer_trx_line_id
                    )
                )
            ),
            0
           ),
           'FM999999999999990D00'
       )                                                                                                                                        TAXABLE_AMOUNT,
       TO_CHAR(
           NVL(
            ABS(
                SUM(
                    (
                        SELECT MAX(CASE WHEN ((dtl.type IN ('INV', 'DM') AND alt.taxable_amount > 0) OR (dtl.type = 'CM' AND alt.taxable_amount < 0)) THEN alt.taxable_amount ELSE 0 END)
                        FROM ra_customer_trx_lines_all  alt,
                             ar_vat_tax_all_vl          tax
                        WHERE alt.vat_tax_id                = tax.vat_tax_id
                        AND   alt.line_type                 = 'TAX'
                        AND   alt.tax_rate                  = 0
                        AND   tax.tax_code LIKE 'IVA%'
                        AND   alt.link_to_cust_trx_line_id 	= dtl.customer_trx_line_id
                    )
                )
            ),
            0
           ),
           'FM999999999999990D00'
       )                                                                                                                                        EXEMPT_AMOUNT,
       TO_CHAR(NVL(ABS(SUM(CASE WHEN dtl.line_type = 'TAX' THEN dtl.tat_extended_amount ELSE 0 END)), 0), 'FM999999999999990D00')               TOTAL_VAT
FROM (
        SELECT dtl.line_type,
               tpy.type,
               dtl.customer_trx_line_id,
               dtl.extended_amount,
               CASE
                   WHEN dtl.line_type = 'LINE' AND ((tpy.type IN ('INV', 'DM') AND dtl.extended_amount > 0) OR (tpy.type = 'CM' AND dtl.extended_amount < 0)) THEN
                       dtl.extended_amount
                   ELSE 
                       0
               END tbt_extended_amount,
               CASE
                WHEN COUNT(*) OVER (PARTITION BY ABS(dtl.tax_rate), NVL(dtl.link_to_cust_trx_line_id, dtl.customer_trx_line_id)) = 1 AND ((tpy.type IN ('INV', 'DM') AND dtl.extended_amount < 0) OR (tpy.type = 'CM' AND dtl.extended_amount > 0)) THEN
                    0
                ELSE
                    dtl.extended_amount
               END tat_extended_amount
        FROM ra_customer_trx_all        trx,
             ra_customer_trx_lines_all  dtl,
             ra_cust_trx_types_all      tpy
        WHERE trx.cust_trx_type_id      = tpy.cust_trx_type_id
        AND   trx.customer_trx_id       = dtl.customer_trx_id
        AND   trx.customer_trx_id       = ?
     ) dtl