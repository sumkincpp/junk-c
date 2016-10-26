int
cache_load(netsnmp_cache * cache, void *vmagic)
{
    std::cout << "_cache_load ... " << std::endl;

    /*DEBUGMSGTL(("internal:ipSystemStatsTable:_cache_load", "called\n"));
    if ((NULL == cache) || (NULL == cache->magic)) {
        snmp_log(LOG_ERR,
                 "invalid cache for ipSystemStatsTable_cache_load\n");
        return -1;
    }

     */
    // should only be called for an invalid or expired cache
    netsnmp_assert((0 == cache->valid) || (1 == cache->expired));

    /*
     * call user code
     */
    return MFD_SUCCESS;
    //return -1;
}                               /* _cache_load */

void
cache_free(netsnmp_cache * cache, void *magic)
{
    std::cout << "_cachefree ... " << std::endl;

}                               /* _cache_free */

static oid      my_test_oid[4] = { 1, 2, 3, 4 };

int
my_test_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    std::cout << "my_test_handler" << std::endl;

    static u_long   accesses = 0;

    accesses++;

    DEBUGMSGTL(("testhandler", "Got request:\n"));
    /*
     * loop through requests
     */
    while (requests) {
        netsnmp_variable_list *var = requests->requestvb;

        DEBUGMSGTL(("testhandler", "  oid:"));
        DEBUGMSGOID(("testhandler", var->name, var->name_length));
        DEBUGMSG(("testhandler", "\n"));

        switch (reqinfo->mode) {
        case MODE_GET:
            if (netsnmp_oid_equals(var->name, var->name_length, my_test_oid, 4)
                == 0) {
                snmp_set_var_typed_value(var, ASN_INTEGER,
                                         (u_char *) & accesses,
                                         sizeof(accesses));
                return SNMP_ERR_NOERROR;
            }
            break;

        case MODE_GETNEXT:
            if (snmp_oid_compare(var->name, var->name_length, my_test_oid, 4)
                < 0) {
                snmp_set_var_objid(var, my_test_oid, 4);
                snmp_set_var_typed_value(var, ASN_INTEGER,
                                         (u_char *) & accesses,
                                         sizeof(accesses));
                return SNMP_ERR_NOERROR;
            }
            break;

        default:
            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_GENERR);
            break;
        }

        requests = requests->next;
    }
    return SNMP_ERR_NOERROR;
}


// Cache handler
netsnmp_handler_registration *reg_handler = netsnmp_create_handler_registration
  ("myTest", my_test_handler, my_test_oid, 4, HANDLER_CAN_RONLY);

netsnmp_register_instance(reg_handler);

std::cout << "netsnmp_cache_create ... " << std::endl;

netsnmp_cache *cache = netsnmp_cache_create(100, cache_load, cache_free, my_test_oid, 4);

assert(cache);
cache->flags = NETSNMP_CACHE_DONT_INVALIDATE_ON_SET |
             NETSNMP_CACHE_DONT_AUTO_RELEASE |
             NETSNMP_CACHE_DONT_FREE_EXPIRED |
             NETSNMP_CACHE_DONT_FREE_BEFORE_LOAD |
             NETSNMP_CACHE_PRELOAD |
             NETSNMP_CACHE_AUTO_RELOAD;

netsnmp_mib_handler *mib_handler  = netsnmp_cache_handler_get(cache);

assert(mib_handler);

netsnmp_cache_handler_owns_cache(mib_handler);
netsnmp_inject_handler(reg_handler, mib_handler);
