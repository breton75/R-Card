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
                          "       ais.dynamic_status_id AS dynamic_status_id, " CR \
                          "       ais.dynamic_utc AS dynamic_utc, " CR \
                          "       gps.init_course AS init_course, " CR \
                          "       gps.init_course_change_ratio AS init_course_change_ratio, " CR \
                          "       gps.init_course_change_segment AS init_course_change_segment, " CR \
                          "       gps.init_speed AS init_speed, " CR \
                          "       gps.init_speed_change_ratio AS init_speed_change_ratio, " CR \
                          "       gps.init_speed_change_segment AS init_speed_change_segment, " CR \
                          "       vessel_types.type_name AS static_vessel_type_name, " CR \
                          "       cargo_types.type_name AS voyage_cargo_type_name, " CR \
                          "       status_types.status_name AS dynamic_status_name " CR \
                          "FROM vessels " CR \
                          "LEFT JOIN gps ON vessels.id = gps.vessel_id " CR \
                          "LEFT JOIN ais ON vessels.id = ais.vessel_id " CR \
                          "LEFT JOIN cargo_types ON ais.voyage_cargo_type_id = cargo_types.id " CR \
                          "LEFT JOIN vessel_types ON ais.static_type_id = vessel_types.id " CR \
                          "LEFT JOIN status_types ON ais.dynamic_status_id = status_types.id  " CR \
                          "WHERE vessels.self = '%1'  " CR \
                          "ORDER BY vessels.id ASC;"

#endif // SQL_DEFS_H
