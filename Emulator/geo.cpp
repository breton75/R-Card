#include "geo.h"

/* длина 1 градуса в зависимости от широты, км */
qreal LAT1DL[91] = 
{111.3, 111.3, 111.3, 111.2, 111.1, 110.9, 110.7, 110.5, 110.2, 110.0,
 109.6, 109.3, 108.9, 108.5, 108.0, 107.6, 107.0, 106.5, 105.9, 105.3,
 104.6, 104.0, 103.3, 102.5, 101.8, 101.0, 100.1,  99.3,  98.4,  97.4,
  96.5,  95.5,  94.5,  93.5,  92.4,  91.3,  90.2,  89.0,  87.8,  86.6,
  85.4,  84.1,  82.9,  81.5,  80.2,  78.8,  77.5,  76.1,  74.6,  73.2,
  71.7,  70.2,  68.7,  67.1,  65.6,  64.0,  62.4,  60.8,  59.1,  57.5,
  55.8,  54.1,  52.4,  50.7,  48.9,  47.2,  45.4,  43.6,  41.8,  40.0,
  38.2,  36.4,  34.5,  32.6,  30.8,  28.9,  27.0,  25.1,  23.2,  21.3,
  19.4,  17.5,  15.5,  13.6,  11.7,   9.7,   7.8,   5.8,   3.9,   1.9, 0.0};
 
/* длина 1 градуса в зависимости от долготы, км */
qreal LON1DL[90] = 
{110.6, 110.6, 110.6, 110.6, 110.6, 110.6, 110.6, 110.6, 110.6, 110.6, 
 110.6, 110.6, 110.6, 110.6, 110.6, 110.7, 110.7, 110.7, 110.7, 110.7, 
 110.7, 110.7, 110.7, 110.8, 110.8, 110.8, 110.8, 110.8, 110.8, 110.8, 
 110.9, 110.9, 110.9, 110.9, 110.9, 111.0, 111.0, 111.0, 111.0, 111.0, 
 111.0, 111.1, 111.1, 111.1, 111.1, 111.1, 111.2, 111.2, 111.2, 111.2, 
 111.2, 111.3, 111.3, 111.3, 111.3, 111.3, 111.4, 111.4, 111.4, 111.4, 
 111.4, 111.4, 111.5, 111.5, 111.5, 111.5, 111.5, 111.5, 111.5, 111.6, 
 111.6, 111.6, 111.6, 111.6, 111.6, 111.6, 111.6, 111.6, 111.7, 111.7, 
 111.7, 111.7, 111.7, 111.7, 111.7, 111.7, 111.7, 111.7, 111.7, 111.7  };

qreal geo::geo2geo_distance(geo::GEOPOSITION& gp1, geo::GEOPOSITION& gp2)
{
  /** http://www.movable-type.co.uk/scripts/latlong.html  ->  Distance **/
  qreal lat1 = qDegreesToRadians(gp1.latitude);
  qreal lat2 = qDegreesToRadians(gp2.latitude);
  qreal dlat = qDegreesToRadians(gp2.latitude - gp1.latitude);
  qreal dlon = qDegreesToRadians(gp2.longtitude - gp1.longtitude);
  
  qreal a = sin(dlat / 2.0) * sin(dlat / 2.0) +
          cos(lat1) * cos(lat2) *
          sin(dlon / 2.0) * sin(dlon / 2.0);
  
  qreal c = 2.0 * atan2(sqrt(a), sqrt(1 - a));
  qreal d = EARTH_RADIUS * c;
//  qDebug() << "dist:" << d;
  
  return d;
}

qreal geo::geo1_geo2_distance(qreal lon1, qreal lat1, qreal lon2, qreal lat2)
{
  /* расстояние между двумя долготами */
  qreal lat = (lat1 + lat2) / 2.0; // средняя широта
  int parallel = int(trunc(lat));
  qreal l1 = LAT1DL[parallel];
  qreal l2 = parallel > 89 ? l1 : LAT1DL[parallel + 1];
  qreal lon_dist = qFabs(lon2 - lon1) * (l1 - ((l1 - l2) * (lat - parallel)));
  
  /* расстояние между двумя широтами */
  qreal lon = (lon1 + lon2) / 2.0; // средняя долгота
  int meridian = int(trunc(lon));
  l1 = LON1DL[meridian];
  l2 = meridian > 89 ? l1 : LON1DL[meridian + 1];
  qreal lat_dist = qFabs(lat2 - lat1) * (l1 - ((l1 - l2) * (lon - meridian)));
  
  /* расстояние между точками. в метрах!!! */
  return qSqrt(qPow(lon_dist, 2.0) + qPow(lat_dist, 2.0)) * 1000; 
  
}

qreal geo::meridian2parallel_km_in_1degree(qreal longtitude, qreal latitude)
{
  qreal lon_km_in_1degree = LON1DL[int(longtitude)];
  qreal lat_km_in_1degree = LAT1DL[int(latitude)];
//  qDebug() << longtitude << lon_km_in_1degree << latitude << lat_km_in_1degree;
  return lon_km_in_1degree / lat_km_in_1degree;
}

bool geo::geoposition_within_bounds(const geo::GEOPOSITION& geopos, geo::BOUNDS* bounds)
{
  return ((geopos.longtitude >= bounds->min_lon) &&
          (geopos.longtitude <= bounds->max_lon) &&
          (geopos.latitude >= bounds->min_lat) &&
          (geopos.latitude <= bounds->max_lat));
}

int geo::get_rnd_course()
{
  QTime t = QTime::currentTime();
  qsrand(t.msecsSinceStartOfDay()); // secsTo(QTime::currentTime()));

  return qrand() % 360;
}

int geo::get_rnd_speed()
{
  QTime t = QTime::currentTime();
  qsrand(t.msecsSinceStartOfDay()); // .secsTo(QTime::currentTime()));
  
  return qrand() % 50; // максимальная скорость 50 узлов
}

geo::COORDINATES geo::get_rnd_coordinates(geo::BOUNDS* bounds)
{
  geo::COORDINATES result;
  
  qreal lat_diff = bounds->max_lat - bounds->min_lat;
  qreal lon_diff = bounds->max_lon - bounds->min_lon;
  
  QTime t = QTime::currentTime();
  qsrand(t.msecsSinceStartOfDay()); // secsTo(QTime::currentTime()));
  qreal r1 = qreal(qrand() % 100) / 100.0;

  result.latitude = bounds->min_lat + r1 * lat_diff;
  
//  qsrand(t.msecsSinceStartOfDay()); // .secsTo(QTime::currentTime()));
  qreal r2 = qreal(qrand() % 100) / 100.0;
  
  result.longtitude = bounds->min_lon + r2 * lon_diff;
  
  return result;
  
}

geo::GEOPOSITION geo::get_rnd_position(geo::BOUNDS* bounds)
{
  geo::GEOPOSITION result;
  
  geo::COORDINATES coord = get_rnd_coordinates(bounds);
  
  result.latitude = coord.latitude;
  result.longtitude = coord.longtitude;
  result.course = get_rnd_course();
  result.speed = get_rnd_speed();
  
  return result;
  
}

geo::GEOPOSITION geo::get_next_geoposition(const geo::GEOPOSITION& geopos, qreal distance)
{
  /** http://www.movable-type.co.uk/scripts/latlong.html **/
  qreal dr = distance / EARTH_RADIUS;
  qreal cr = qDegreesToRadians(qreal(geopos.course));
  qreal lat1 = qDegreesToRadians(geopos.latitude);
  
  qreal lat2 = asin(sin(lat1) * cos(dr) + cos(lat1) * sin(dr) * cos(cr));
  qreal lon2 = atan2(sin(cr) * sin(dr) * cos(lat1), cos(dr) - sin(lat1) * sin(lat2));

  geo::GEOPOSITION new_geopos = geopos;  
  new_geopos.latitude = qRadiansToDegrees(lat2);
  new_geopos.longtitude += qRadiansToDegrees(lon2);
  
  return new_geopos;
}

qreal geo::lon2lon_distance(qreal min_lon, qreal max_lon, qreal lat)
{
  geo::GEOPOSITION gp1(min_lon, lat, 0, 0);
  geo::GEOPOSITION gp2(max_lon, lat, 0, 0);
  
  qreal gp1gp2_dist = geo::geo2geo_distance(gp1, gp2);
  
  return gp1gp2_dist;
}

qreal geo::lat2lat_distance(qreal min_lat, qreal max_lat, qreal lon)
{
  geo::GEOPOSITION gp1(lon, min_lat, 0, 0);
  geo::GEOPOSITION gp2(lon, max_lat, 0, 0);
  
  qreal gp1gp2_dist = geo::geo2geo_distance(gp1, gp2);
  
  return gp1gp2_dist;
}

//void getGridStep(AREA_DATA* area_data)
//{
//  /* считаем в метрах */
////  qreal area_meters = 1000 * lon1_lon2_distance(area_data->geo_bounds.min_lon, area_data->geo_bounds.max_lon, area_data->geo_bounds.max_lat);
//  qreal area_meters = 1000 * lat1_lat2_distance(area_data->geo_bounds.min_lat, area_data->geo_bounds.max_lat, area_data->geo_bounds.max_lon);
  
//  int line_count = area_data->area_curr_size.width() / MINOR_LINE_INTERVAL;
//  area_data->gridCellDistance = quint64(trunc(area_meters / line_count));
  
//  while(area_data->gridCellDistance % 50)
//    area_data->gridCellDistance++;
  
//  area_data->gridCellStep = area_data->gridCellDistance *area_data->area_curr_size.width() / area_meters;
  
//}
