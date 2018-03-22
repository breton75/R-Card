#ifndef SQL_DEFS_H
#define SQL_DEFS_H

#define CR "\n"

#define SQL_SELECT_VESSELS "SELECT vessels.id AS id, " CR \
                           "       vessels.self AS self, " CR \
                           "       ais.static_callsign AS static_callsign, " CR \
                           "       ais.static_imo AS static_imo, " CR \
                           "       ais.static_mmsi AS static_mmsi, " CR \
                           "       ais.static_type_id AS static_type_id, " CR \
                           "       ais.static_length AS static_length, " CR \
                           "       ais.static_width AS static_width, " CR \
                           "       ais.voyage_destination AS voyage_destination, " CR \
                           "       ais.voyage_draft AS voyage_draft, " CR \
                           "       ais.voyage_cargo_type_id AS voyage_cargo_type_id, " CR \
                           "       ais.voyage_team AS voyage_team, " CR \
                           "       ais.dynamic_course AS dynamic_course, " CR \
                           "       ais.dynamic_latitude AS dynamic_latitude, " CR \
                           "       ais.dynamic_longtitude AS dynamic_longtitude, " CR \
                           "       ais.dynamic_speed AS dynamic_speed, " CR \
                           "       ais.dynamic_utc AS dynamic_utc, " CR \
                           "       ais.nav_status_id AS nav_status_id, " CR \
                           "       gps.timeout AS gps_timeout, " CR \
                           "       gps.init_random_coordinates AS init_random_coordinates, " CR \
                           "       gps.init_random_course AS init_random_course, " CR \
                           "       gps.init_random_speed AS init_random_speed," CR \
                           "       gps.init_course_change_ratio AS init_course_change_ratio, " CR \
                           "       gps.init_course_change_segment AS init_course_change_segment, " CR \
                           "       gps.init_speed_change_ratio AS init_speed_change_ratio, " CR \
                           "       gps.init_speed_change_segment AS init_speed_change_segment, " CR \
                           "       gps.last_update AS gps_last_update, " CR \
                           "       vessel_types.type_name AS static_vessel_type_name, " CR \
                           "       cargo_types.type_name AS voyage_cargo_type_name, " CR \
                           "       nav_statuses.status_name AS nav_status_name " CR \
                           "FROM vessels " CR \
                           "LEFT JOIN gps ON vessels.id = gps.vessel_id " CR \
                           "LEFT JOIN ais ON vessels.id = ais.vessel_id " CR \
                           "LEFT JOIN cargo_types ON ais.voyage_cargo_type_id = cargo_types.id " CR \
                           "LEFT JOIN vessel_types ON ais.static_type_id = vessel_types.id " CR \
                           "LEFT JOIN nav_statuses ON ais.nav_status_id = nav_statuses.id  "

#define SQL_WHERE_SELF "WHERE vessels.self = %1 "
#define SQL_WHERE_ID   "WHERE vessels.id = %1 "
#define SQL_WHERE_LAST_INSERTED "WHERE vessels.id = last_insert_rowid();"


#define SQL_ORDER_BY_VESSELS_ID_ASC "ORDER BY vessels.id ASC;"
#define SQL_ORDER_BY_VESSELS_ID_DESC "ORDER BY vessels.id DESC;"


#define SQL_SELECT_VESSELS_WHERE_SELF (SQL_SELECT_VESSELS SQL_WHERE_SELF SQL_ORDER_BY_VESSELS_ID_ASC)

#define SQL_SELECT_VESSEL_WHERE_ID (SQL_SELECT_VESSELS SQL_WHERE_ID)

#define SQL_SELECT_LAST_INSERTED_VESSEL (SQL_SELECT_VESSELS SQL_WHERE_LAST_INSERTED)


#define SQL_INSERT_NEW_VESSEL "INSERT INTO vessels (self) VALUES (%1);"

#define SQL_INSERT_NEW_AIS "INSERT INTO ais (vessel_id, " CR \
                           "                 static_mmsi, " CR \
                           "                 static_imo, " CR \
                           "                 static_type_id, " CR \
                           "                 static_callsign, " CR \
                           "                 static_length, " CR \
                           "                 static_width, " CR \
                           "                 voyage_destination, " CR \
                           "                 voyage_draft, " CR \
                           "                 voyage_cargo_type_id, " CR \
                           "                 voyage_team)  " CR \
                           "VALUES ((select id from vessels order by id desc limit 1), " CR \
                           "        '%1', '%2', %3, '%4', %5, %6, '%7', %8, %9, %10);"

#define SQL_INSERT_NEW_GPS "INSERT INTO gps (vessel_id," CR \
                           "                 timeout," CR \
                           "                 init_random_coordinates," CR \
                           "                 init_random_course," CR \
                           "                 init_random_speed," CR \
                           "                 init_course_change_ratio," CR \
                           "                 init_speed_change_ratio," CR \
                           "                 init_course_change_segment," CR \
                           "                 init_speed_change_segment," CR \
                           "                 last_update)" CR \
                           "VALUES ((select id from vessels order by id desc limit 1), " CR \
                           "        %1, '%2', '%3', '%4', %5, %6, %7, %8, " CR \
                           "        strftime('%Y-%m-%d %H:%M:%f', 'now', 'localtime'));"


#define SQL_UPDATE_AIS "UPDATE ais SET static_mmsi = %1, " CR \
                       "               static_imo = %2, " CR \
                       "               static_type_id = %3, " CR \
                       "               static_callsign = '%4', " CR \
                       "               static_length = %5, " CR \
                       "               static_width = %6, " CR \
                       "               voyage_destination = '%7', " CR \
                       "               voyage_draft = %8, " CR \
                       "               voyage_cargo_type_id = %9, " CR \
                       "               voyage_team = %10  " CR \
                       "WHERE vessel_id = %11 "


#define SQL_UPDATE_GPS "UPDATE gps SET timeout = %1," CR \
                       "               init_random_coordinates = '%2'," CR \
                       "               init_random_course = '%3'," CR \
                       "               init_random_speed = '%4'," CR \
                       "               init_course_change_ratio = %5," CR \
                       "               init_speed_change_ratio = %6," CR \
                       "               init_course_change_segment = %7," CR \
                       "               init_speed_change_segment = %8," CR \
                       "               last_update = strftime('%Y-%m-%d %H:%M:%f', 'now', 'localtime')" CR \
                       "WHERE vessel_id = %9"


#define SQL_SELECT_VESSEL_TYPES "SELECT id, type_name FROM vessel_types;"
#define SQL_SELECT_CARGO_TYPES "SELECT id, type_name FROM cargo_types;"
#define SQL_SELECT_NAV_STATS "select id, status_name, static_voyage_interval, dynamic_interval from nav_statuses"
#define SQL_SELECT_LAG_TYPES "select id, type_name from lag_types"


#define SQL_SELECT_COUNT_FROM_SERIAL_WHERE  "SELECT count() as count " \
                                      "FROM serial_port_params WHERE device_type = %1"

#define SQL_INSERT_SERIAL  "INSERT INTO serial_port_params (device_type) VALUES(%1)"

#define SQL_UPDATE_SERIAL_WHERE  "UPDATE serial_port_params SET port_name='%1', baudrate=%2, "\
                                 "parity=%3, stop_bits=%4, data_bits=%5, flow_control=%6 WHERE device_type = %7"

#endif // SQL_DEFS_H
