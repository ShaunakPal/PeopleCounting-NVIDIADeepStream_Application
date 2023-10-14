//
// Created by kamoliddin on 9/23/23.
//

#ifndef DS_SCH_PEOPLE_COUNT_MSG_BUILD_H
#define DS_SCH_PEOPLE_COUNT_MSG_BUILD_H
#include <gst/gst.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include </usr/local/cuda-12/include/cuda_runtime_api.h>
#include <sys/timeb.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <glib-2.0/glib/gstrfuncs.h>
#include <glib-2.0/glib/gmem.h>
#include <glib-2.0/glib/gutils.h>
#include "gstnvdsmeta.h"
#include "nvdsmeta_schema.h"
#include "nvds_yml_parser.h"
#include "nvds_analytics_meta.h"
#include "nvdsanalytics_meta_utils.h"

#include "gst-nvmessage.h"
#include "analytics.h"

//void  generate_event_msg_meta(gpointer data, AnalyticsUserMeta *user_meta);

void generate_event_msg_meta(gpointer data, gint class_id);
extern gpointer  meta_copy_func(gpointer data, gpointer user_data);

extern void meta_free_func(gpointer data, gpointer user_data);
#endif //DS_SCH_PEOPLE_COUNT_MSG_BUILD_H
