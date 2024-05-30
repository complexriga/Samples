WITH param AS (SELECT ? CUSTOMER_TRX_ID FROM dual)
SELECT trx.trx_number                                                                                                                           REF_NUMBER,
       TO_CHAR(trx.trx_date, 'YYYY-MM-DD')                                                                                                      REF_DATE
FROM ar_receivable_applications_all app,
     ra_customer_trx_all            trx,
     ra_cust_trx_types_all          tpy,
     param
WHERE app.applied_customer_trx_id   = trx.customer_trx_id
AND   trx.cust_trx_type_id          = tpy.cust_trx_type_id
AND   app.customer_trx_id           = param.customer_trx_id
UNION
SELECT trx.trx_number                                                                                                                           REF_NUMBER,
       TO_CHAR(trx.trx_date, 'YYYY-MM-DD')                                                                                                      REF_DATE
FROM ra_customer_trx_all            app,
     ra_customer_trx_all            trx,
     ra_cust_trx_types_all          tpy,
     param
WHERE app.previous_customer_trx_id  = trx.customer_trx_id
AND   trx.cust_trx_type_id          = tpy.cust_trx_type_id
AND   app.customer_trx_id           = param.customer_trx_id