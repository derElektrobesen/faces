#ifndef MAIN_H
#define MAIN_H

#include <QDebug>

#define DB_NAME     "faces"
#define DB_F_NAME   "faces.db3"

#ifndef USE_LOW_LEVEL_CONVERSION
#   ifndef USE_HIGH_LEVEL_CONVERSION
#       define USE_HIGH_LEVEL_CONVERSION
#   endif
#elif defined USE_HIGH_LEVEL_CONVERSION
#   undef USE_HIGH_LEVEL_CONVERSION
#endif

#define st_arr_len(__arr__) ( sizeof(__arr__) / sizeof(*__arr__) )
#define st_arr_len_s(__arr__) ((ssize_t)st_arr_len(__arr__))

#define DECLARE_SQL_CON(var) QSqlQuery var(QSqlDatabase::database(DB_NAME))

#endif // MAIN_H
