/*
 * util/json_helper.c - helper for parsing json input to Connection
 *
 * Copyright (c) 2016, NLnet Labs. All rights reserved.
 *
 * This software is open source.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This file contains functions needed for proper parsing json input as Connection
 * 
 */

#include "json_helper.h"
#include <string.h>

/**
 * Parses input json char* to ConnectionChain
 * @Example //TODO:
 *
 * @param input
 * @return ConnectionChain*
 */
ConnectionChain* parseConnections(char* input) {
    printf(input);
    printf("\n");

    if (!input || !json_validate(input)) {
        handleErr();

    }
    JsonNode *head = json_decode(input);
    //shouldn't be null because we checked if it is valid
    if (head->tag != JSON_OBJECT) {
        handleErr();

    }

    JsonNode *node = head->children.head; // now it should be the first dictionary value e.g. connections

    if (!node || strcmp(node->key, "connections") != 0) { // and also must be array
        handleErr();

    }
    // node is array called connections

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Connection *con;
    ConnectionChain *cons = newConnectionChain();
    JsonNode *connection = node->children.head;

    while (NULL != connection) {
        con = newConnection();
        JsonNode *param = connection->children.head;
        //zpracovani paramu

        while (NULL != param) {
            if (param->tag == JSON_BOOL && strcmp(param->key, "default") == 0) {
                con->default_con = param->bool_; // hope it copies the value
            } else if (param->tag == JSON_STRING && strcmp(param->key, "type") == 0) {
                if (strcmp(param->string_, "wifi") == 0) {
                    con->type = WIFI;
                } else if (strcmp(param->string_, "vpn") == 0) {
                    con->type = VPN;
                } else if (strcmp(param->string_, "other") == 0) {
                    con->type = OTHER;
                } else {
                    con->type = IGNORE;
                }
            } else if (param->tag == JSON_ARRAY && strcmp(param->key, "zones") == 0) {
                JsonNode *zone = param->children.head;
                CharChain *zn = newCharChain();

                while (NULL != zone) {
                    char *znc = calloc(strlen(zone->string_) + 1, sizeof (char));
                    if (NULL == znc)
                        outOfMemory();
                    strcpy(znc, zone->string_);
                    charChain_append(zn, znc);

                    zone = zone->next;
                }
                con->zones = zn;
            } else if (param->tag == JSON_ARRAY && strcmp(param->key, "servers") == 0) {
                JsonNode *server = param->children.head;
                CharChain *sv = newCharChain();

                while (NULL != server) {
                    char *svc = calloc(strlen(server->string_) + 1, sizeof (char));
                    if (NULL == svc)
                        outOfMemory();
                    strcpy(svc, server->string_);
                    charChain_append(sv, svc);

                    server = server->next;
                }
                con->servers = sv;
            }

            param = param->next;

        }
        connection = connection->next;
        connectionChain_append(cons, con);
    }

    json_delete(head);
    return cons;
}

void printType(JsonNode *node) {
    if (node->tag == JSON_ARRAY)
        printf("array\n");
    else if (node->tag == JSON_STRING)
        printf("string\n");
    else if (node->tag == JSON_OBJECT)
        printf("object\n");
}

void handleErr() {
    printf("bad json input");
    exit(-1);
}

CharChain* newCharChain() {
    CharChain *cn = calloc(1, sizeof (CharChain));
    if (NULL == cn) {
        outOfMemory();
    }
    return cn;
}

void freeCharChain(CharChain *cn) {
    if (NULL != cn->next) {
        freeCharChain(cn->next);
        //free(cn->next);
    }
    free(cn->current);

    //if(NULL == cn->prev)
    free(cn);
}

ConnectionChain* newConnectionChain() {
    ConnectionChain *cn = calloc(1, sizeof (ConnectionChain));
    if (NULL == cn) {
        outOfMemory();
    }
    return cn;
}

void freeConnectionChain(ConnectionChain *cn, bool preserveValues) { // we're freeing in just one direction
    if (NULL != cn->next) {
        freeConnectionChain(cn->next, preserveValues);
        free(cn->next);
    }

    if (!preserveValues && NULL != cn->current)
        freeConnection(cn->current);

    if (NULL == cn->prev)
        free(cn);
}

Connection* newConnection() {
    Connection *con = calloc(1, sizeof (Connection));
    if (NULL == con) {
        outOfMemory();
    }
    return con;
}

void freeConnection(Connection *con) {
    if (con != NULL) {
        // default_con & type doesn't have to be freed/ mustn't be freed they weren't allocated by malloc
        freeCharChain(con->servers);
        freeCharChain(con->zones);
        free(con);
    }
}

void structPrint(Connection *con) {
    if (con == NULL)
        return;

    if (con->default_con) {
        printf("default: true\n");
    } else {
        printf("default: false\n");
    }
    printf("type: ");
    switch (con->type) {
        case WIFI:
            printf("wifi\n");
            break;

        case VPN:
            printf("vpn\n");
            break;

        default:
        case OTHER:
            printf("other\n");
            break;

    }

    printf("servers: \n");
    for (CharChain *i = con->servers; i != NULL; i = i->next) {
        printf(i->current);
        printf("\n");
    }

    printf("\n");
    printf("zones: \n");
    for (CharChain *i = con->zones; i != NULL; i = i->next) {
        printf(i->current);
        printf("\n");
    }
    printf("\n");
}

void outOfMemory() {
    printf("OUT OF MEMORY!");
    exit(-1);
}

void charChain_append_double(CharChain **head, char *value) {
    if (*head == NULL) {
        CharChain *chain = newCharChain();
        chain->current = value;
        *head = chain;
        return;
    }

    CharChain *tmp = *head;
    if (NULL == tmp->current) {
        tmp->current = value;
        tmp->prev = NULL;
        tmp->next = NULL;
        return;
    }

    while (NULL != tmp->next) {
        tmp = tmp->next;
    }

    CharChain *vl = newCharChain();
    vl->current = value;
    vl->prev = tmp;
    tmp->next = vl;

}

CharChain* charChain_append(CharChain *chain, char *value) {
    if (chain == NULL) {
        chain = newCharChain();
        chain->current = value;
        return chain;
    }

    CharChain *tmp = chain;
    if (NULL == tmp->current) {
        tmp->current = value;
        tmp->prev = NULL;
        tmp->next = NULL;
        return chain;
    }

    while (NULL != tmp->next) {
        tmp = tmp->next;
    }

    CharChain *vl = newCharChain();
    vl->current = value;
    vl->prev = tmp;
    tmp->next = vl;

    return chain;

}

ConnectionChain* connectionChain_append(ConnectionChain *chain, Connection *value) {
    ConnectionChain *tmp = chain;
    if (NULL == tmp->current) {
        tmp->current = value;
        tmp->prev = NULL;
        tmp->next = NULL;
        return chain;
    }

    while (NULL != tmp->next) {
        tmp = tmp->next;
    }

    ConnectionChain *vl = newConnectionChain();
    vl->current = value;
    vl->prev = tmp;
    vl->next = NULL;
    tmp->next = vl;

    return chain;
}

//char** charChain_to_array(CharChain *chain) {
//    CharChain *tmp = chain;
//    int length = 1; // because we need one for null pointer as an array delimiter
//
//    if (NULL != chain->current) {
//        length++;
//    }
//
//    while (tmp->next) {
//        tmp = tmp->next;
//        length++;
//    }
//
//    char **arry = calloc(length, sizeof (char*));
//    if (NULL == arry) {
//        outOfMemory();
//    }
//
//    tmp = chain;
//    for (int i = 0; i < length - 2; i++) {
//        arry[i] = tmp->current;
//        tmp = tmp->next;
//    }
//
//    return arry;
//}
//
//Connection* connectionChain_to_array(ConnectionChain *chain) {
//    ConnectionChain *tmp = chain;
//    int length = 1; // because we need one for null pointer as an array delimiter
//
//    if (NULL != chain->current) {
//        length++;
//    }
//
//    while (tmp->next) {
//        tmp = tmp->next;
//        length++;
//    }
//
//    Connection *arry = calloc(length, sizeof (Connection*));
//    if (NULL == arry) {
//        outOfMemory();
//    }
//
//    tmp = chain;
//    for (int i = 0; i < length - 2; i++) {
//        arry[i] = *tmp->current;
//        tmp = tmp->next;
//    }
////    Connection *nl = calloc(1, sizeof(Connection));
////    nl->type = DELIMITER;
////    arry[length] = *nl;
//    return arry;
//}

//ConnectionChain* onlyVPN(ConnectionChain *cn) {
//	ConnectionChain *toRet = newConnectionChain();
//
//	for(ConnectionChain *i = cn; i != NULL; i = i->next) {
//		if(i->current->type == VPN) {
//			connectionChain_append(toRet, i->current);
//		}
//	}
//	return toRet;
//}

void freeConnectionChainCell(ConnectionChain *cn) {
    freeConnection(cn->current);
    free(cn);
}

ConnectionChain* onlyVPN(ConnectionChain *cn) {
    ConnectionChain *top = cn;
    ConnectionChain *i = cn;
    while (i != NULL) {
        if (i->current->type != VPN) {
            if (i->prev != NULL)
                i->prev->next = i->next;
            if (i->next != NULL)
                i->next->prev = i->prev;
            if (i->next != NULL || i->prev != NULL) {// means that cell isn't the only one in chain
                if (i->prev == NULL)
                    top = i->next;
                ConnectionChain *tmp = i->next;
                freeConnectionChainCell(i);
                i = tmp;
                continue;
            } else {
                if (i->current != NULL) {
                    freeConnection(i->current);
                    i->current = NULL;
                }
            }
        }
        i = i->next;
    }

    return top;
}

ConnectionChain* onlyDefault(ConnectionChain *cn) {
    ConnectionChain *top = cn;
    ConnectionChain *i = cn;

    while (i != NULL) {
        if (!i->current->default_con) {
            if (i->prev != NULL)
                i->prev->next = i->next;
            if (i->next != NULL)
                i->next->prev = i->prev;
            if (i->next != NULL || i->prev != NULL) {// means that cell isn't the only one in chain
                if (i->prev == NULL)
                    top = i->next;
                ConnectionChain *tmp = i->next;
                freeConnectionChainCell(i);
                i = tmp;
                continue;
            } else {
                if (i->current != NULL) {
                    freeConnection(i->current);
                    i->current = NULL;
                }
            }
        }
        i = i->next;
    }

    return top;
}

bool isEmpty(ConnectionChain *cn) { // only one direction checking
    if (NULL == cn || (NULL == cn->current && NULL == cn->next))
        return true;
    return false;
}

bool valueInCharChain(CharChain *cn, char* value) {
    if (NULL == value) {
        return false;
    }
    for (CharChain *i = cn; NULL != i; i = i->next) {
        if (NULL == i->current)
            return false;
        if (strcmp(i->current, value) == 0)
            return true;
    }

    return false;
}

int charChainLength(CharChain *cn) {
    int len = 0;

    CharChain *tmp = cn;
    while (NULL != tmp) {
        tmp = tmp->next;
        len++;
    }

    return len;
}

bool charChainsEqual(CharChain *cn1, CharChain *cn2) { // not the order of values, just "is it also there?"
    if (NULL == cn1 && NULL == cn2)
        return true;

    if ((NULL == cn1 && NULL != cn2) || (NULL == cn2 && NULL != cn1))
        return false;

    if (charChainLength(cn1) != charChainLength(cn2)) { // we pretend every value is unique
        return false;
    }

    for (CharChain *i = cn1; NULL != i->next; i = i->next) {
        if (!valueInCharChain(cn2, i->current))
            return false;
    }

    return true;

}

CharChain* copy_CharChain(CharChain *cn) {
    if (cn == NULL)
        return NULL;

    CharChain *toRet = newCharChain();


    CharChain *nxt = NULL;
    if (cn->next != NULL) {
        nxt = copy_CharChain(cn->next);
    }
    toRet->next = nxt;

    if (cn->current != NULL) {
        char *chr = calloc(strlen(cn->current) + 1, sizeof (char));
        if (chr == NULL) {
            outOfMemory();
        }

        strcpy(chr, cn->current);
        toRet->current = chr;
    }

    if (toRet->next != NULL) {
        toRet->next->prev = toRet;
    }
    return toRet;
}

Connection* copy_Connection(Connection* cn) {

    if (cn == NULL) {
        return NULL;
    }
    Connection *toRet = newConnection();


    toRet->default_con = cn->default_con;
    toRet->type = cn->type;
    toRet->servers = copy_CharChain(cn->servers);
    toRet->zones = copy_CharChain(cn->zones);

    return toRet;
}

ConnectionChain* copy_ConnectionChain(ConnectionChain *cn) {

    if (cn == NULL)
        return NULL;

    ConnectionChain *toRet = newConnectionChain();


    ConnectionChain *nxt = NULL;
    if (cn->next != NULL) {
        nxt = copy_ConnectionChain(cn->next);
    }
    toRet->next = nxt;

    if (cn->current != NULL) {
        toRet->current = copy_Connection(cn->current);
    }

    if (toRet->next != NULL) {
        toRet->next->prev = toRet;
    }
    return toRet;
}

ConnectionChain* noWifi(ConnectionChain *cn) {
    ConnectionChain *top = cn;
    ConnectionChain *i = cn;
    while (i != NULL) {
        if (i->current->type == WIFI) {
            if (i->prev != NULL)
                i->prev->next = i->next;
            if (i->next != NULL)
                i->next->prev = i->prev;
            if (i->next != NULL || i->prev != NULL) {// means that cell isn't the only one in chain
                if (i->prev == NULL)
                    top = i->next;
                ConnectionChain *tmp = i->next;
                freeConnectionChainCell(i);
                i = tmp;
                continue;
            } else {
                if (i->current != NULL) {
                    freeConnection(i->current);
                    i->current = NULL;
                }
            }
        }
        i = i->next;
    }

    return top;
}

Connection* getPreferredConnection(Connection *first, Connection *second) //todo: add IPv6 functionality of default_con
{
    if (second->type == VPN && first->type != VPN) {
        return second;
    }

    if (first->type == VPN && second->type != VPN) {
        return first;
    }

    if (second->default_con && !first->default_con) {
        return second;
    }

    if (first->default_con && !second->default_con) {
        return first;
    }

    return first;
}

AssocChain* newAssocChain() {
    AssocChain *cn = calloc(1, sizeof (AssocChain));

    if (cn == NULL) {
        outOfMemory();
    }

    return cn;
}

void freeAssocChain(AssocChain *cn, bool preserveValues) { // we're freeing in just one direction

    if (NULL != cn->next) {
        freeAssocChain(cn->next, preserveValues);
        free(cn->next);
    }

    if (!preserveValues && NULL != cn->zone)
        free(cn->zone);

    if (!preserveValues && NULL != cn->connection)
        freeConnection(cn->connection);

    if (NULL == cn->prev)
        free(cn);

}

AssocChain* assocChain_append(AssocChain *cn, char *zone, Connection *con) {
    AssocChain *tmp = cn;
    if (NULL == tmp->zone && NULL == tmp->connection) {
        tmp->zone = zone;
        tmp->connection = con;
        tmp->prev = NULL;
        tmp->next = NULL;
        return cn;
    }

    while (NULL != tmp->next) {
        tmp = tmp->next;
    }

    AssocChain *vl = newAssocChain();
    vl->zone = zone;
    vl->connection = con;
    vl->prev = tmp;
    vl->next = NULL;
    tmp->next = vl;

    return cn;
}

AssocChain* getAssocChainWithZone(AssocChain *cn, char *zone) {
    for (AssocChain *i = cn; i != NULL; i = i->next) {
        if (i->zone == NULL)
            continue;
        if (strcmp(i->zone, zone) == 0) {
            return i;
        }
    }
    return NULL;
}

AssocChain* getZoneConnectionMapping(ConnectionChain *connections) {
    AssocChain *result = newAssocChain();
    for (ConnectionChain *i = connections; i != NULL; i = i->next) {
        //TODO:null handle
        for (CharChain *l = i->current->zones; l != NULL; l = l->next) {
            AssocChain *tmp = getAssocChainWithZone(result, l->current);
            if (tmp == NULL) {
                assocChain_append(result, l->current, i->current);
            } else {
                tmp->connection = getPreferredConnection(tmp->connection, i->current);
            }
        }
    }
    return result;
}

ZoneConfig* newZoneConfig() {
    ZoneConfig *zc = calloc(1, sizeof (ZoneConfig));

    if (zc == NULL)
        outOfMemory();

    return zc;
}

void freeZoneConfig(ZoneConfig *zn, bool preserveValues) {
    if (NULL != zn->next) {
        freeZoneConfig(zn->next, preserveValues);
        free(zn->next);
    }

    if (!preserveValues && NULL != zn->ips)
        freeCharChain(zn->ips);

    if (!preserveValues && NULL != zn->name)
        free(zn->name);

    //		if(!preserveValues && NULL != zn->secure)
    //			free(zn->secure);

    if (NULL == zn->prev)
        free(zn);
}

ZoneConfig* zoneConfig_append(ZoneConfig *what, ZoneConfig *where) {
    ZoneConfig *tmp = where;
    if (NULL == tmp->name && NULL == tmp->ips) {
        tmp->name = what->name;
        tmp->ips = what->ips;
        tmp->secure = what->secure;
        tmp->prev = NULL;
        tmp->next = NULL;

        freeZoneConfig(what, true);
        return where;
    }

    while (NULL != tmp->next) {
        tmp = tmp->next;
    }

    what->prev = tmp;
    tmp->next = what;

    return where;
}

ZoneConfig* getUnboundZoneConfig() {
    // what if too many requests like at probing for example?
    FILE *unbound = popen("unbound-control status", "r");
    pclose(unbound); //maybe the EXIT_SUCCESS condition would be great here to determine if unbound is running correctly

    unbound = popen("unbound-control list_forwards", "r");
    // subprocess.check_output(["unbound-control", "list_forwards"]).decode() -- decode is missing and is not implemented here

    char *buf = NULL;
    char word_buf[512] = {0};
    int word_buf_pos = 0;
    size_t len = 0;

    ZoneConfig *zoneConfig = newZoneConfig();
    ZoneConfig *tmp = newZoneConfig();
    CharChain *chtmp = newCharChain();

    int pos_in_fields = 1;

    while (getline(&buf, &len, unbound) != -1) { //deprecated comment: sometimes some chars are missing don't know why
        for (int i = 0; i < len; i++) {
            if ((buf[i] == ' ' || buf[i] == '\n') && word_buf_pos > 0) {
                if (pos_in_fields == 1) {
                    char *nm = calloc(word_buf_pos + 1, sizeof (char));
                    if (nm == NULL)
                        outOfMemory();
                    strncpy(nm, word_buf, word_buf_pos - 1); // we don't want a dot at the end of a string
                    tmp->name = nm;

                    pos_in_fields++;
                    word_buf_pos = 0;

                } else if (pos_in_fields == 2 || pos_in_fields == 3) {
                    // should be "IN" or "FORWARD" value we don't want them so skip them
                    pos_in_fields++;
                    word_buf_pos = 0;

                } else if (pos_in_fields == 4) {
                    if (strncmp(word_buf, "+i", 2) == 0) {
                        tmp->secure = false;
                    } else {
                        tmp->secure = true;
                        char *ip = calloc(word_buf_pos + 1, sizeof (char));
                        if (ip == NULL) {
                            outOfMemory();
                        }
                        strncpy(ip, word_buf, word_buf_pos);

                        charChain_append(chtmp, ip);
                        //means its ip address
                    }

                    pos_in_fields++;
                    word_buf_pos = 0;
                } else if (pos_in_fields > 4) { // pos is greater
                    char *ip = calloc(word_buf_pos + 1, sizeof (char));
                    if (ip == NULL) {
                        outOfMemory();
                    }
                    strncpy(ip, word_buf, word_buf_pos);

                    charChain_append(chtmp, ip);

                    pos_in_fields++;
                    word_buf_pos = 0;

                }
            } else if (buf[i] == ' ' && word_buf_pos == 0) { // ignore leading spaces if any
                //do nothing
            } else {
                word_buf[word_buf_pos] = *(buf + i); // copy the char value
                word_buf_pos++;
            }
            if (buf[i] == '\n') {
                tmp->ips = chtmp;
                chtmp = newCharChain();
                zoneConfig_append(tmp, zoneConfig);
                tmp = newZoneConfig();

                pos_in_fields = 1;
                word_buf_pos = 0;
                break;
            }
        }
    }

    // za predpokladu, ze cely vypis konci novym radkem jinak mazeme posledni zaznam nikoliv prazdnou bunku pripravenou k plneni
    freeZoneConfig(tmp, false); //values may be null
    if (buf != NULL)
        free(buf);
    freeCharChain(chtmp);
    pclose(unbound);

    return zoneConfig;
}

LocalZoneConfig* newLocalZoneConfig() {
    LocalZoneConfig *cn = calloc(1, sizeof (LocalZoneConfig));
    if (NULL == cn) {
        outOfMemory();
    }
    return cn;
}

void freeLocalZoneConfig(LocalZoneConfig *cfg, bool preserveValues) {
    if (NULL != cfg->next) {
        freeLocalZoneConfig(cfg->next, preserveValues);
        free(cfg->next);
    }

    if (!preserveValues && NULL != cfg->name)
        free(cfg->name);

    if (!preserveValues && NULL != cfg->type)
        free(cfg->type);

    if (NULL == cfg->prev)
        free(cfg);
}

LocalZoneConfig* localZoneConfig_append(LocalZoneConfig *what, LocalZoneConfig *where) {
    LocalZoneConfig *tmp = where;
    if (NULL == tmp->name && NULL == tmp->type) {
        tmp->name = what->name;
        tmp->type = what->type;
        tmp->prev = NULL;
        tmp->next = NULL;

        freeLocalZoneConfig(what, true);
        return where;
    }

    while (NULL != tmp->next) {
        tmp = tmp->next;
    }

    what->prev = tmp;
    tmp->next = what;

    return where;
}

LocalZoneConfig* getUnboundLocalZoneConfig() {
    // what if too many requests like at probing for example?
    FILE *unbound = popen("unbound-control status", "r");
    pclose(unbound); //maybe the EXIT_SUCCESS condition would be great here to determine if the unbound is running correctly

    unbound = popen("unbound-control list_local_zones", "r");
    // subprocess.check_output(["unbound-control", "list_forwards"]).decode() -- decode is missing and is not implemented here

    char *buf = NULL;
    char word_buf[512] = {0};
    int word_buf_pos = 0;
    size_t len = 0;


    LocalZoneConfig *localZoneConfig = newLocalZoneConfig();
    LocalZoneConfig *tmp = newLocalZoneConfig();

    int pos_in_fields = 1;

    while (getline(&buf, &len, unbound) != -1) {//deprecated comment: sometimes some chars are missing don't know why
        for (int i = 0; i < len; i++) {
            if ((buf[i] == ' ' || buf[i] == '\n') && word_buf_pos > 0) {
                if (pos_in_fields == 1) {
                    char *nm = calloc(word_buf_pos + 1, sizeof (char));
                    if (nm == NULL)
                        outOfMemory();
                    strncpy(nm, word_buf, word_buf_pos - 1); // we don't want a dot at the end of a string
                    tmp->name = nm;

                    pos_in_fields++;
                    word_buf_pos = 0;

                } else if (pos_in_fields == 2) {
                    char *tp = calloc(word_buf_pos + 1, sizeof (char));
                    if (tp == NULL)
                        outOfMemory();
                    strncpy(tp, word_buf, word_buf_pos);
                    tmp->type = tp;

                    pos_in_fields++;
                    word_buf_pos = 0;

                }
            } else if (buf[i] == ' ' && word_buf_pos == 0) {
                // do nothing
            } else {
                word_buf[word_buf_pos] = *(buf + i); // hope it copies char value
                word_buf_pos++;
            }

            if (buf[i] == '\n') {
                localZoneConfig_append(tmp, localZoneConfig);
                tmp = newLocalZoneConfig();

                pos_in_fields = 1;
                word_buf_pos = 0;
                break;
            }
        }
    }

    // za predpokladu, ze cely vypis konci novym radkem jinak mazeme zaznam nikoliv prazdnou bunku pripravenou k plneni
    freeLocalZoneConfig(tmp, false); //values may be null
    pclose(unbound);
    if (buf != NULL)
        free(buf);
    return localZoneConfig;
}

bool isEmptyCharChain(CharChain *cc) {
    if (NULL == cc || (NULL == cc->current && NULL == cc->next))
        return true;
    return false;
}

void unbound_local_zones_add(char *zone, char *type) {
    int len = 27;
    len += strlen(zone);
    len++; // one space
    len += strlen(type);
    len++; //string delimiter
    char *command = calloc(len, sizeof (char));
    if (command == NULL)
        outOfMemory();
    strcpy(command, "unbound-control local_zone ");
    strcat(command, zone);
    strcat(command, " ");
    strcat(command, type);
    FILE *ubd = popen(command, "r");
    //we can read any answer here
    pclose(ubd);
    free(command);
}

void unbound_zones_remove(char *zone, char *flush_command) {
    int len = 31;
    len += strlen(zone);
    len++; //string delimiter
    char *command = calloc(len, sizeof (char));
    if (command == NULL)
        outOfMemory();
    strcpy(command, "unbound-control forward_remove ");
    strcat(command, zone);
    FILE *ubd = popen(command, "r");
    //we can read any answer here
    pclose(ubd);
    free(command);

    len = 16;
    len += strlen(flush_command);
    len++; // one space
    len += strlen(zone);
    len++; //string delimiter
    command = calloc(len, sizeof (char));
    if (command == NULL)
        outOfMemory();

    strcpy(command, "unbound-control ");
    strcat(command, flush_command);
    strcat(command, " ");
    strcat(command, zone);

    ubd = popen(command, "r");
    // here we can read any answer
    pclose(ubd);
    free(command);
    ubd = popen("unbound-control flush_requestlist", "r");
    pclose(ubd);
}

void freeCharChainCell(CharChain *cc) {
    free(cc->current);
    free(cc);
}

void stored_zones_remove_double(CharChain **chain, char* zone) { //removes only the first occurrence
    for (CharChain **i = chain; *i != NULL;) {
        CharChain *entry = *i;

        if (strcmp(entry->current, zone) == 0) {
            *i = entry->next;
            freeCharChainCell(entry);
            return;
        }

        i = &entry->next;
    }
}

//void stored_zones_remove(char *zone) {
//    CharChain *i = stored_zones;
//    while (i != NULL) {
//        if (i->current == NULL) {
//            i = i->next;
//            continue;
//        }
//        if (strcmp(i->current, zone) == 0) {
//            if (i->prev != NULL)
//                i->prev->next = i->next;
//            if (i->next != NULL)
//                i->next->prev = i->prev;
//            if (i->next != NULL || i->prev != NULL) {// means that cell isn't the only one in chain
//                CharChain *tmp = i->next; //todo: use new ** concept
//                //				if(i->prev != NULL && i->prev->prev == NULL)
//                //					stored_zones = i->prev;
//                freeCharChainCell(i);
//                i = tmp;
//                continue;
//            } else {
//                if (i->current != NULL) {
//                    free(i->current);
//                    i->current = NULL;
//                }
//            }
//        }
//        i = i->next;
//    }
//}

char* servers_to_string(CharChain *servers) {
    int len = 0;

    for (CharChain *tmp = servers; tmp != NULL; tmp = tmp->next) {
        if (tmp->current != NULL) {
            len += strlen(tmp->current);
            len++; // one space
        }
    } // includes string delimiter because there isn't a space after the last word

    char *toRet = calloc(len, sizeof (char));
    if (toRet == NULL)
        outOfMemory();

    for (CharChain *tmp = servers; tmp != NULL; tmp = tmp->next) {
        if (tmp->prev == NULL) {
            strcpy(toRet, tmp->current);

        } else {
            strcat(toRet, tmp->current);
        }

        if (tmp->next != NULL) {
            strcat(toRet, " ");
        }
    }
    return toRet;
}

void unbound_zones_add(char *zone, CharChain *servers, bool validate) {
    int len = 28;
    len += 3; // for "+i " if any
    len += strlen(zone);
    len++; // one space
    char *srvs = servers_to_string(servers); // don't forget to deallocate this
    len += strlen(srvs);
    len++; //string delimiter
    char *command = calloc(len, sizeof (char));
    if (command == NULL)
        outOfMemory();
    strcpy(command, "unbound-control forward_add ");
    if (!validate) {
        strcat(command, "+i ");
    }
    strcat(command, zone);
    strcat(command, " ");
    strcat(command, srvs);
    FILE *ubd = popen(command, "r");
    //we can read any answer here
    pclose(ubd);
    free(command);
    free(srvs);
}

void unbound_local_zones_remove(char *zone) {
    int len = 34;
    len += strlen(zone);
    len++; // string delimiter

    char *command = calloc(len, sizeof (char));

    if (command == NULL)
        outOfMemory();

    strcpy(command, "unbound-control local_zone_remove ");
    strcat(command, zone);

    FILE *ubd = popen(command, "r");
    pclose(ubd);
    free(command);
}