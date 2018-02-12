#ifndef SQL_DEFS_H
#define SQL_DEFS_H

#define CR "\n"

#define SQL_SELCT_VESSELS "select " CR \
                           "  vessels.id as id, " CR \
                           "  vessels.callsign as callsign, " CR \
                           "  vessels.destination as destination, " CR \
                           "  vessels.draft as draft, " CR \
                           "  vessels.cargo_type_id as cargo_type_id, " CR \
                           "  vessels.imo as imo, " CR \
                           "  vessels.mmsi as mmsi, " CR \
                           "  vessels.length as length, " CR \
                           "  vessels.width as width, " CR \
                           "  vessels.self as self, " CR \
                           "  vessels.team as team, " CR \
                           "  vessels.type_id as type_id, " CR \
                           "  vessels.init_course as init_course, " CR \
                           "  vessels.init_course_change_ratio as init_course_change_ratio, " CR \
                           "  vessels.init_course_change_segment as init_course_change_segment, " CR \
                           "  vessels.init_speed as init_speed, " CR \
                           "  vessels.init_speed_change_ratio as init_speed_change_ratio, " CR \
                           "  vessels.init_speed_change_segment as init_speed_change_segment, " CR \
                           "  vessels.dynamic_course as dynamic_course, " CR \
                           "  vessels.dynamic_latitude as dynamic_latitude, " CR \
                           "  vessels.dynamic_longtitude as dynamic_longtitude, " CR \
                           "  vessels.dynamic_status_id as status_id, " CR \
                           "  vessels.dynamic_utc as dynamic_utc, " CR \
                           "  vessel_types.type_name as vessel_type_name, " CR \
                           "  cargo_types.type_name as cargo_type_name, " CR \
                           "  status_types.status_name as status_name  " CR \
                           "from vessels " CR \
                           "  left join cargo_types on vessels.cargo_type_id = cargo_types.id " CR \
                           "  left join vessel_types on vessels.type_id = vessel_types.id " CR \
                           "  left join status_types on vessels.status_id = status_types.id " CR \
                           "where vessels.self = '%1' " CR \
                           "order by vessels.id asc"

#endif // SQL_DEFS_H
