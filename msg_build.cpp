//
// Created by kamoliddin on 9/23/23.
//

#include "msg_build.h"
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

#define MAX_TIME_STAMP_LEN 32
#define PGIE_CLASS_ID_VEHICLE 2
#define PGIE_CLASS_ID_PERSON 0

static void
generate_vehicle_meta (gpointer data)
{
    NvDsVehicleObject *obj = (NvDsVehicleObject *) data;

    obj->type = g_strdup ("sedan");
    obj->color = g_strdup ("blue");
    obj->make = g_strdup ("Bugatti");
    obj->model = g_strdup ("M");
    obj->license = g_strdup ("XX1234");
    obj->region = g_strdup ("CA");
}

static void
generate_person_meta (gpointer data)
{
    NvDsPersonObject *obj = (NvDsPersonObject *) data;
    obj->age = 45;
    obj->cap = g_strdup ("none");
    obj->hair = g_strdup ("black");
    obj->gender = g_strdup ("male");
    obj->apparel = g_strdup ("formal");
}

static void
generate_ts_rfc3339(char *buf, int buf_size) {
    time_t tloc;
    struct tm tm_log;
    struct timespec ts;
    char strmsec[6];              //.nnnZ\0

    clock_gettime(CLOCK_REALTIME, &ts);
    memcpy(&tloc, (void *) (&ts.tv_sec), sizeof(time_t));
    gmtime_r(&tloc, &tm_log);
    strftime(buf, buf_size, "%Y-%m-%dT%H:%M:%S", &tm_log);
    int ms = ts.tv_nsec / 1000000;
    g_snprintf(strmsec, sizeof(strmsec), ".%.3dZ", ms);
    strncat(buf, strmsec, buf_size);
}

//extern void
//generate_event_msg_meta(void *data,
//                        AnalyticsUserMeta *analytics_meta) {
//    NvDsEventMsgMeta *meta = (NvDsEventMsgMeta *) data;
//    meta->sensorId = 0;
//    meta->placeId = 0;
//    meta->moduleId = 0;
//    meta->sensorStr = g_strdup("sensor-0");
//
//    meta->ts = (char *) g_malloc0(MAX_TIME_STAMP_LEN + 1);
//
////    meta->objectId = (gchar *) g_malloc0(MAX_LABEL_SIZE);
////
////    strncpy(meta->objectId, obj_params->obj_label, MAX_LABEL_SIZE);
//
//    generate_ts_rfc3339(meta->ts, MAX_TIME_STAMP_LEN);
//
//    meta->type = NVDS_EVENT_MOVING;
//    meta->objType = NVDS_OBJECT_TYPE_VEHICLE;
//    meta->objClassId = PGIE_CLASS_ID_VEHICLE;
//
//}

extern void
generate_event_msg_meta (gpointer data, gint class_id)
{
    NvDsEventMsgMeta *meta = (NvDsEventMsgMeta *) data;


    meta->ts = (gchar *) g_malloc0 (MAX_TIME_STAMP_LEN + 1);
    meta->objectId = (gchar *) g_malloc0 (MAX_LABEL_SIZE);

    strncpy (meta->objectId, "1", MAX_LABEL_SIZE);

    generate_ts_rfc3339 (meta->ts, MAX_TIME_STAMP_LEN);


    /*
     * This demonstrates how to attach custom objects.
     * Any custom object as per requirement can be generated and attached
     * like NvDsVehicleObject / NvDsPersonObject. Then that object should
     * be handled in payload generator library (nvmsgconv.cpp) accordingly.
     */
    if (class_id == PGIE_CLASS_ID_VEHICLE) {
        meta->type = NVDS_EVENT_MOVING;
        meta->objType = NVDS_OBJECT_TYPE_VEHICLE;
        meta->objClassId = PGIE_CLASS_ID_VEHICLE;

        NvDsVehicleObject *obj =
                (NvDsVehicleObject *) g_malloc0 (sizeof (NvDsVehicleObject));
        generate_vehicle_meta (obj);

        meta->extMsg = obj;
        meta->extMsgSize = sizeof (NvDsVehicleObject);
    } else if (class_id == PGIE_CLASS_ID_PERSON ) {
        meta->objType = NVDS_OBJECT_TYPE_PERSON;
        meta->objClassId = PGIE_CLASS_ID_PERSON;

        NvDsPersonObject *obj =
                (NvDsPersonObject *) g_malloc0 (sizeof (NvDsPersonObject));
        generate_person_meta (obj);

        meta->extMsg = obj;
        meta->extMsgSize = sizeof (NvDsPersonObject);
    }
}

extern gpointer
meta_copy_func(gpointer data, gpointer user_data) {
    NvDsUserMeta *user_meta = (NvDsUserMeta *) data;
    NvDsEventMsgMeta *srcMeta = (NvDsEventMsgMeta *) user_meta->user_meta_data;
    NvDsEventMsgMeta *dstMeta = NULL;

    dstMeta = static_cast<NvDsEventMsgMeta *>(g_memdup(srcMeta, sizeof(NvDsEventMsgMeta)));

    if (srcMeta->ts)
        dstMeta->ts = g_strdup(srcMeta->ts);

    if (srcMeta->sensorStr)
        dstMeta->sensorStr = g_strdup(srcMeta->sensorStr);

    if (srcMeta->objSignature.size > 0) {
        dstMeta->objSignature.signature = static_cast<gdouble *>(g_memdup(srcMeta->objSignature.signature,
                                                                          srcMeta->objSignature.size));
        dstMeta->objSignature.size = srcMeta->objSignature.size;
    }

    if (srcMeta->objectId) {
        dstMeta->objectId = g_strdup(srcMeta->objectId);
    }

    if (srcMeta->extMsgSize > 0) {
        if (srcMeta->objType == NVDS_OBJECT_TYPE_VEHICLE) {
            NvDsVehicleObject *srcObj = (NvDsVehicleObject *) srcMeta->extMsg;
            NvDsVehicleObject *obj =
                    (NvDsVehicleObject *) g_malloc0(sizeof(NvDsVehicleObject));
            if (srcObj->type)
                obj->type = g_strdup(srcObj->type);
            if (srcObj->make)
                obj->make = g_strdup(srcObj->make);
            if (srcObj->model)
                obj->model = g_strdup(srcObj->model);
            if (srcObj->color)
                obj->color = g_strdup(srcObj->color);
            if (srcObj->license)
                obj->license = g_strdup(srcObj->license);
            if (srcObj->region)
                obj->region = g_strdup(srcObj->region);

            dstMeta->extMsg = obj;
            dstMeta->extMsgSize = sizeof(NvDsVehicleObject);
        } else if (srcMeta->objType == NVDS_OBJECT_TYPE_PERSON) {
            NvDsPersonObject *srcObj = (NvDsPersonObject *) srcMeta->extMsg;
            NvDsPersonObject *obj =
                    (NvDsPersonObject *) g_malloc0(sizeof(NvDsPersonObject));

            obj->age = srcObj->age;

            if (srcObj->gender)
                obj->gender = g_strdup(srcObj->gender);
            if (srcObj->cap)
                obj->cap = g_strdup(srcObj->cap);
            if (srcObj->hair)
                obj->hair = g_strdup(srcObj->hair);
            if (srcObj->apparel)
                obj->apparel = g_strdup(srcObj->apparel);

            dstMeta->extMsg = obj;
            dstMeta->extMsgSize = sizeof(NvDsPersonObject);
        }
    }
    return dstMeta;
}

extern void
meta_free_func(gpointer data, gpointer user_data) {
    NvDsUserMeta *user_meta = (NvDsUserMeta *) data;
    NvDsEventMsgMeta *srcMeta = (NvDsEventMsgMeta *) user_meta->user_meta_data;

    g_free(srcMeta->ts);
    g_free(srcMeta->sensorStr);

    if (srcMeta->objSignature.size > 0) {
        g_free(srcMeta->objSignature.signature);
        srcMeta->objSignature.size = 0;
    }

    if (srcMeta->objectId) {
        g_free(srcMeta->objectId);
    }

    if (srcMeta->extMsgSize > 0) {
        if (srcMeta->objType == NVDS_OBJECT_TYPE_VEHICLE) {
            NvDsVehicleObject *obj = (NvDsVehicleObject *) srcMeta->extMsg;
            if (obj->type)
                g_free(obj->type);
            if (obj->color)
                g_free(obj->color);
            if (obj->make)
                g_free(obj->make);
            if (obj->model)
                g_free(obj->model);
            if (obj->license)
                g_free(obj->license);
            if (obj->region)
                g_free(obj->region);
        } else if (srcMeta->objType == NVDS_OBJECT_TYPE_PERSON) {
            NvDsPersonObject *obj = (NvDsPersonObject *) srcMeta->extMsg;

            if (obj->gender)
                g_free(obj->gender);
            if (obj->cap)
                g_free(obj->cap);
            if (obj->hair)
                g_free(obj->hair);
            if (obj->apparel)
                g_free(obj->apparel);
        }
        g_free(srcMeta->extMsg);
        srcMeta->extMsgSize = 0;
    }
    g_free(user_meta->user_meta_data);
    user_meta->user_meta_data = NULL;
}