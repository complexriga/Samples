SELECT tpy.type                                                                                                                                 TRX_TYPE,
       trx.trx_number                                                                                                                           TRX_NUMBER,
       TO_CHAR(trx.trx_date, 'YYYY-MM-DD')                                                                                                      TRX_DATE,
       TO_CHAR(trx.trx_date, 'HH24:MI:SS')                                                                                                      TRX_TIME,
       trx.invoice_currency_code                                                                                                                TRX_CURRENCY,
       TO_CHAR(
                NVL(
                    ARPT_SQL_FUNC_UTIL.GET_FIRST_REAL_DUE_DATE(
                                        trx.customer_trx_id,
                                        trx.term_id,
                                        trx.trx_date
                    ),
                    trx.trx_date
                ),
                'YYYY-MM-DD'
       )                                                                                                                                        TRX_DUE_DATE,
       (
         SELECT trm.name
         FROM ra_terms_vl trm
         WHERE trm.term_id  = trx.term_id
       )                                                                                                                                        TRX_TERM,
       (
        SELECT dst.attribute1||dst.attribute2||dst.attribute3||dst.attribute4||dst.attribute5||dst.attribute6||dst.attribute7||dst.attribute8
        FROM ra_customer_trx_lines_all dst
        WHERE dst.customer_trx_id   = trx.customer_trx_id
        AND   dst.line_number       = 1
        AND   dst.line_type         = 'LINE'
       )                                                                                                                                        TRX_DESCRIPTION,
       trx.comments                                                                                                                             TRX_COMMENTS,
       (
        SELECT TO_CHAR(NVL(ABS(SUM(dst.extended_amount)), 0), 'FM999999999999990D00')
        FROM ra_customer_trx_lines_all dst
        WHERE dst.customer_trx_id   = trx.customer_trx_id
        AND   dst.line_type         = 'LINE'
        AND   dst.extended_amount * CASE WHEN tpy.type IN ('INV', 'DM') THEN -1 ELSE 1 END > 0
       )                                                                                                                                        TRX_WARRANTY,
       (
        SELECT TO_CHAR(ABS(SUM(dtl.extended_amount)), 'FM999999999999990D00')
        FROM ra_customer_trx_lines_all dtl
        WHERE dtl.customer_trx_id = trx.customer_trx_id
       )                                                                                                                                        TRX_TOTAL,
       SUBSTR(org.registration_number, 1, INSTR(org.registration_number, '-') - 1)                                                              SELLER_TAXPAYER_ID,
       SUBSTR(org.registration_number, -1, 1)                                                                                                   SELLER_CHECK_DIGIT,
       org.name                                                                                                                                 SELLER_NAME,
       loc.address_line_1                                                                                                                       SELLER_ADDRESS,
       loc.region_2                                                                                                                             SELLER_DEPARTMENT_NAME,
       (
            SELECT gi.identifier_value
            FROM hz_geographies             g_state,
                 hz_geography_identifiers   gi
            WHERE g_state.geography_id          = gi.geography_id
            AND   g_state.geography_type        = 'STATE'
            AND   g_state.country_code          = 'CO'
            AND   gi.identifier_subtype         = 'CLL_F041_CODE'
            AND   g_state.geography_element2    = loc.region_2
       )                                                                                                                                        SELLER_DEPARTMENT_CODE,
       loc.town_or_city                                                                                                                         SELLER_CITY_NAME,
       (
            SELECT gi.identifier_value
            FROM hz_geographies            g_city,
                 hz_geography_identifiers  gi
            WHERE g_city.geography_id       = gi.geography_id
            AND   g_city.geography_element3 = loc.town_or_city
            AND   g_city.country_code       = 'CO'
            AND   g_city.geography_type     = 'CITY'
            AND   gi.identifier_subtype     = 'CLL_F041_CODE'
            AND   g_city.geography_element2 = loc.region_2
            AND   g_city.geography_element3 = loc.town_or_city
       )                                                                                                                                        SELLER_CITY_CODE,
       (
            SELECT g_country.geography_element1
            FROM hz_geographies             g_country,
                 hz_geography_identifiers   gi
            WHERE g_country.geography_id        = gi.geography_id
            AND   g_country.geography_type      = 'COUNTRY'
            AND   gi.identifier_subtype         = 'CLL_F041_CODE'
            AND   gi.identifier_value           = SUBSTR(loc.style, 1, 2)
       )                                                                                                                                        SELLER_COUNTRY_NAME,
       SUBSTR(loc.style, 1, 2)                                                                                                                  SELLER_COUNTRY_CODE,
       loc.address_line_1||', '||loc.town_or_city||', '||loc.region_2||', '||SUBSTR(loc.style, 1, 2)                                            SELLER_ADDRESS_DESC,
       (
        SELECT alt.name
        FROM jtf_rs_salesreps alt
        WHERE alt.org_id        = trx.org_id
        AND   alt.salesrep_id   = trx.primary_salesrep_id
       )                                                                                                                                        SELLER_VENDOR_NAME,
       (
        SELECT gcc.segment5
        FROM ra_customer_trx_lines_all    dtl,
             ra_cust_trx_line_gl_dist_all dst,
             gl_code_combinations         gcc
        WHERE dtl.customer_trx_line_id = dst.customer_trx_line_id
        AND   dst.code_combination_id  = gcc.code_combination_id
        AND   dtl.line_number          = 1
        AND   dtl.line_type            = 'LINE'
        AND   dtl.customer_trx_id      = trx.customer_trx_id
       )                                                                                                                                        SELLER_PROJECT,
       (
        SELECT gcc.segment6
        FROM ra_customer_trx_lines_all    dtl,
             ra_cust_trx_line_gl_dist_all dst,
             gl_code_combinations         gcc
        WHERE dtl.customer_trx_line_id = dst.customer_trx_line_id
        AND   dst.code_combination_id  = gcc.code_combination_id
        AND   dtl.line_number          = 1
        AND   dtl.line_type            = 'LINE'
        AND   dtl.customer_trx_id      = trx.customer_trx_id
       )                                                                                                                                        SELLER_FRONT,
       nit.nit                                                                                                                                  CUSTOMER_ID,
       nit.verifying_digit                                                                                                                      CUSTOMER_CHECK_DIGIT,
       prt.party_name                                                                                                                           CUSTOMER_NAME,
       prt.person_first_name                                                                                                                    CUSTOMER_FIRST_NAME,
       prt.person_last_name                                                                                                                     CUSTOMER_LAST_NAME,
       hlc.address1                                                                                                                             CUSTOMER_ADDRESS,
       hlc.state                                                                                                                                CUSTOMER_DEPARTMENT_NAME,
       (
        SELECT gi.identifier_value
        FROM hz_geographies             g_state,
             hz_geography_identifiers   gi
        WHERE g_state.geography_id          = gi.geography_id
        AND   g_state.geography_type        = 'STATE'
        AND   g_state.country_code          = 'CO'
        AND   gi.identifier_subtype         = 'CLL_F041_CODE'
        AND   g_state.geography_element2    = hlc.state
       )                                                                                                                                        CUSTOMER_DEPARTMENT_CODE,
       hlc.city                                                                                                                                 CUSTOMER_CITY_NAME,
       (
            SELECT gi.identifier_value
            FROM hz_geographies             g_city,
                  hz_geography_identifiers  gi
            WHERE g_city.geography_id       = gi.geography_id
            AND   g_city.geography_element3 = loc.town_or_city
            AND   g_city.country_code       = 'CO'
            AND   g_city.geography_type     = 'CITY'
            AND   gi.identifier_subtype     = 'CLL_F041_CODE'
            AND   g_city.geography_element2 = hlc.state
            AND   g_city.geography_element3 = hlc.city
       )                                                                                                                                        CUSTOMER_CITY_CODE,
       (
        SELECT alt2.identifier_value
        FROM hz_geography_identifiers alt1,
             hz_geography_identifiers alt2
        WHERE alt1.geography_id         = alt2.geography_id
        AND   alt1.geography_type       = 'COUNTRY'
        AND   alt1.identifier_type      = 'CODE'
        AND   alt1.identifier_subtype   = 'CLL_F041_CODE'
        AND   alt2.identifier_type      = 'NAME'
        AND   alt1.identifier_value     = hlc.country
       )                                                                                                                                        CUSTOMER_COUNTRY_NAME,
       hlc.country                                                                                                                              CUSTOMER_COUNTRY_CODE,
       (
        SELECT alt.party_name
        FROM hz_relationships       rls,
             hz_cust_account_roles  rol,
             hz_parties             alt,
             hz_cust_acct_sites_all cas,
             hz_party_site_uses     psu
        WHERE rls.object_id             = alt.party_id
        AND   rls.party_id              = rol.party_id
        AND   rls.subject_id            = prt.party_id
        AND   rol.cust_acct_site_id     = cas.cust_acct_site_id
        AND   cas.party_site_id         = psu.party_site_id
        AND   psu.party_site_use_id     = trx.bill_to_site_use_id
        AND   rls.subject_table_name    = 'HZ_PARTIES'
        AND   rls.object_table_name     = 'HZ_PARTIES'
        AND   rls.relationship_code     = 'CONTACT'
        AND   rls.directional_flag      = 'B'
        AND   rls.status                = 'A'
        AND   SYSDATE BETWEEN rls.start_date AND rls.end_date
        AND   rol.primary_flag          = 'Y'
        AND   rol.status                = 'A'
       )                                                                                                                                        CUSTOMER_CONTACT,
       (
        SELECT cpt.email_address
        FROM hz_relationships       rls,
             hz_cust_account_roles  rol,
             hz_cust_acct_sites_all cas,
             hz_party_site_uses     psu,
             hz_contact_points      cpt
        WHERE rls.party_id              = cpt.owner_table_id
        AND   rls.party_id              = rol.party_id
        AND   rls.subject_id            = prt.party_id
        AND   rol.cust_acct_site_id     = cas.cust_acct_site_id
        AND   cas.party_site_id         = psu.party_site_id
        AND   psu.party_site_use_id     = trx.bill_to_site_use_id
        AND   rls.subject_table_name    = 'HZ_PARTIES'
        AND   rls.object_table_name     = 'HZ_PARTIES'
        AND   rls.relationship_code     = 'CONTACT'
        AND   rls.directional_flag      = 'B'
        AND   rls.status                = 'A'
        AND   SYSDATE BETWEEN rls.start_date AND rls.end_date
        AND   cpt.contact_point_type    = 'EMAIL'
        AND   cpt.owner_table_name      = 'HZ_PARTIES'
        AND   cpt.status                = 'A'
        AND   cpt.primary_flag          = 'Y'
        AND   rol.primary_flag          = 'Y'
        AND   rol.status                = 'A'
       )                                                                                                                                        CUSTOMER_CONTACT_EMAIL,
       (
        SELECT cpt.phone_number
        FROM hz_relationships       rls,
             hz_cust_account_roles  rol,
             hz_cust_acct_sites_all cas,
             hz_party_site_uses     psu,
             hz_contact_points      cpt
        WHERE rls.party_id              = cpt.owner_table_id
        AND   rls.party_id              = rol.party_id
        AND   rls.subject_id            = prt.party_id
        AND   rol.cust_acct_site_id     = cas.cust_acct_site_id
        AND   cas.party_site_id         = psu.party_site_id
        AND   psu.party_site_use_id     = trx.bill_to_site_use_id
        AND   rls.subject_table_name    = 'HZ_PARTIES'
        AND   rls.object_table_name     = 'HZ_PARTIES'
        AND   rls.relationship_code     = 'CONTACT'
        AND   rls.directional_flag      = 'B'
        AND   rls.status                = 'A'
        AND   SYSDATE BETWEEN rls.start_date AND rls.end_date
        AND   cpt.contact_point_type    = 'PHONE'
        AND   cpt.owner_table_name      = 'HZ_PARTIES'
        AND   cpt.status                = 'A'
        AND   cpt.primary_flag          = 'Y'
        AND   rol.primary_flag          = 'Y'
        AND   rol.status                = 'A'
       )                                                                                                                                        CUSTOMER_CONTACT_PHONE,
       trx.exchange_rate                                                                                                                        EXCHANGE_RATE,
       TO_CHAR(trx.exchange_date, 'YYYY-MM-DD')                                                                                                 EXCHANGE_DATE,
       'COP'                                                                                                                                    EXCHANGE_TO_CURRENCY,
       trx.ct_reference                                                                                                                         CT_REFERENCE,
       trx.customer_trx_id
FROM ra_customer_trx_all        trx,
     ra_cust_trx_types_all      tpy,
     (
        SELECT DISTINCT 
               hrl.country,
               hroutl_bg.NAME bg,
               lep.legal_entity_id,
               lep.name,
               hroutl_ou.NAME ou_name,
               hroutl_ou.organization_id,
               hrl.location_id,
               hrl.location_code,
               glev.flex_segment_value,
               reg.registration_number
        FROM xle_entity_profiles          lep,
             xle_registrations            reg,
             hr_locations_all             hrl,
             hz_parties                   hzp,
             fnd_territories_vl           ter,
             hr_operating_units           hro,
             hr_all_organization_units_vl hroutl_bg,
             hr_all_organization_units_vl hroutl_ou,
             hr_organization_units        gloperatingunitseo,
             gl_legal_entities_bsvs       glev
        WHERE lep.party_id                       = hzp.party_id
        AND   lep.legal_entity_id                = reg.source_id
        AND   hrl.location_id                    = reg.location_id
        AND   ter.territory_code                 = hrl.country
        AND   lep.legal_entity_id                = hro.default_legal_context_id
        AND   gloperatingunitseo.organization_id = hro.organization_id
        AND   hroutl_bg.organization_id          = hro.business_group_id
        AND   hroutl_ou.organization_id          = hro.organization_id
        AND   glev.legal_entity_id               = lep.legal_entity_id
        AND   lep.transacting_entity_flag        = 'Y'
        AND   reg.source_table                   = 'XLE_ENTITY_PROFILES'
        AND   reg.identifying_flag               = 'Y'
     )                          org,
     hr_locations_all           loc,
     hz_parties                 prt,
     hz_cust_accounts_all       cst,
     hz_cust_site_uses_all      csu,
     hz_cust_acct_sites_all     cas,
     hz_party_sites             pst,
     hz_locations               hlc,
     jl_co_gl_nits              nit,
     ra_batch_sources_all       src
WHERE trx.cust_trx_type_id      = tpy.cust_trx_type_id
AND   trx.org_id                = org.organization_id
AND   org.location_id           = loc.location_id
AND   trx.bill_to_customer_id   = cst.cust_account_id
AND   cst.party_id              = prt.party_id
AND   prt.jgzz_fiscal_code      = nit.nit
AND   trx.bill_to_site_use_id   = csu.site_use_id
AND   csu.cust_acct_site_id     = cas.cust_acct_site_id
AND   cas.party_site_id         = pst.party_site_id
AND   pst.location_id           = hlc.location_id
AND   trx.batch_source_id       = src.batch_source_id
AND   trx.complete_flag         = 'Y'
AND   tpy.type IN ('INV', 'CM', 'DM')
AND   trx.customer_trx_id       = ?