/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   json_data_types.h
 * Author: fcap
 *
 * Created on 26. srpna 2016, 11:20
 */

#ifndef JSON_DATA_TYPES_H
#define JSON_DATA_TYPES_H

typedef struct connection Connection;
typedef struct charChain CharChain;
typedef enum type Type;
typedef struct connectionChain ConnectionChain;
typedef struct assocChain AssocChain;
typedef struct zoneConfig ZoneConfig;
typedef struct localZoneConfig LocalZoneConfig;

enum type {
    VPN,
    WIFI,
    OTHER,
    IGNORE,
    DELIMITER
};

struct assocChain {
    char *zone;
    Connection *connection;
    AssocChain *prev;
    AssocChain *next;
};

struct connection {
    int default_con;
    CharChain *zones;
    Type type;
    CharChain *servers;
};

struct charChain {
    CharChain *prev;
    char *current;
    CharChain *next;
};

struct connectionChain {
    ConnectionChain *prev;
    Connection *current;
    ConnectionChain *next;
};

struct zoneConfig {
    ZoneConfig *prev;
    ZoneConfig *next;

    char *name;
    CharChain *ips;
    int secure;
};

struct localZoneConfig {
    LocalZoneConfig *prev;
    LocalZoneConfig *next;

    char *name;
    char *type; // in future the type could be ENUM
};

#endif /* JSON_DATA_TYPES_H */

