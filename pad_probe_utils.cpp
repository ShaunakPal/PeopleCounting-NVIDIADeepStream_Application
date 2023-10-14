//
// Created by kamoliddin on 9/23/23.
//

#include "analytics.h"
#include "gst-nvmessage.h"
#include "msg_build.h"
#include "nvdsanalytics_meta_utils.h"
#include "nvds_analytics_meta.h"
#include "nvds_yml_parser.h"
#include "nvdsmeta_schema.h"
#include "gstnvdsmeta.h"
#include <sstream>
#include <vector>
#include <iostream>
#include <math.h>
#include <sys/timeb.h>
#include </usr/local/cuda-12/include/cuda_runtime_api.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "nvbufsurface.h"
#include "nvds_obj_encode.h"
#include <glib.h>
#include <gst/gst.h>
#include <curl/curl.h>
#include <pthread.h>
#include "pad_probe_utils.h"
#include "msg_build.h"
#include <string>
#include <cstring>
#include <algorithm>

#define PGIE_CLASS_ID_VEHICLE 2
#define PGIE_CLASS_ID_PERSON 0
#define save_img TRUE
#define attach_user_meta TRUE

gint frame_number = 0, frame_count = 0;

void *sendFilePath(void *arg) {

    const char *filepath = (const char *) arg;


    CURL *curl;
    CURLcode res;


    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] = "Content-Type: multipart/form-data";
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        // Set the URL for the POST request
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/upload");

        // Add the file to the POST request
        curl_formadd(&formpost, &lastptr,
                     CURLFORM_COPYNAME, "image",
                     CURLFORM_FILE, filepath,
                     CURLFORM_CONTENTTYPE, "image/jpeg",
                     CURLFORM_END);
        // Set the form data
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

        // Set the HTTP headers
        headerlist = curl_slist_append(headerlist, buf);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

        // Set the response callback function
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
//        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&writeData);

        // Perform the POST request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            g_printerr("curl_easy_perform() failed: %s\n",
                       curl_easy_strerror(res));
        }
        // Cleanup
        curl_easy_cleanup(curl);
        curl_formfree(formpost);
        curl_slist_free_all(headerlist);

    }
    // Cleanup global curl resources
    curl_global_cleanup();
    pthread_exit(NULL);

}

void callSendFilePathAsynchronously(const char *filepath) {
    pthread_t thread;
    pthread_attr_t attr;

// copy filepath to a new string
    char *filepath_copy = (char *) malloc(strlen(filepath) + 1);
    strcpy(filepath_copy, filepath);

    // Initialize thread attributes
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // Create the thread and pass the file path
    if (pthread_create(&thread, &attr, sendFilePath, (void *) filepath_copy) != 0) {
        fprintf(stderr, "Error creating thread\n");
    }


    // Clean up the thread attributes
    pthread_attr_destroy(&attr);
    // Clean up filepath_copy
    free(filepath_copy);

}
//std::vector<int> object_ids= {};

/* nvdsanalytics_src_pad_buffer_probe  will extract metadata received on tiler sink pad
 * and extract nvanalytics metadata etc. */
extern GstPadProbeReturn
nvdsanalytics_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info,
                                   gpointer ctx) {
    GstBuffer *buf = (GstBuffer *) info->data;
    GstMapInfo inmap = GST_MAP_INFO_INIT;
    if (!gst_buffer_map(buf, &inmap, GST_MAP_READ)) {
        GST_ERROR ("input buffer mapinfo failed");
        return GST_PAD_PROBE_DROP;
    }

    NvDsObjectMeta *obj_meta = NULL;

    guint person_count = 0;
    NvBufSurface *ip_surf = (NvBufSurface *) inmap.data;
    gst_buffer_unmap(buf, &inmap);
    NvDsMetaList *l_frame = NULL;
    NvDsMetaList *l_obj = NULL;
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);

    for (l_frame = batch_meta->frame_meta_list; l_frame != NULL;
         l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) (l_frame->data);

        gboolean screen_shot_not_taken = TRUE;
//        std::stringstream out_string;
        std::stringstream lane_string;


        guint num_rects = 0;
        for (l_obj = frame_meta->obj_meta_list; l_obj != NULL; l_obj = l_obj->next) {
            obj_meta = (NvDsObjectMeta *) (l_obj->data);
            for (NvDsMetaList *l_user_meta = obj_meta->obj_user_meta_list; l_user_meta != NULL;
                 l_user_meta = l_user_meta->next) {
                NvDsUserMeta *user_meta = (NvDsUserMeta *) (l_user_meta->data);

                if (user_meta->base_meta.meta_type == NVDS_USER_OBJ_META_NVDSANALYTICS) {
                    NvDsAnalyticsObjInfo *user_meta_data = (NvDsAnalyticsObjInfo *) user_meta->user_meta_data;
                    std::vector<std::string> lane(user_meta_data->lcStatus);
                    for (int i = 0; i < lane.size(); i++) {
                        lane_string << lane[i];
                    }

                    if (PGIE_CLASS_ID_PERSON == obj_meta->class_id && lane_string.str() == "Entry") {

                        g_print("Person crossed the line\n");
                        g_print(lane_string.str().c_str());
                        NvDsEventMsgMeta *msg_meta =
                                (NvDsEventMsgMeta *) g_malloc0(sizeof(NvDsEventMsgMeta));
                        msg_meta->bbox.top = obj_meta->rect_params.top;
                        msg_meta->bbox.left = obj_meta->rect_params.left;
                        msg_meta->bbox.width = obj_meta->rect_params.width;
                        msg_meta->bbox.height = obj_meta->rect_params.height;
                        msg_meta->frameId = frame_meta->frame_num;
                        msg_meta->trackingId = obj_meta->object_id;
                        msg_meta->confidence = obj_meta->confidence;

                        msg_meta->sensorId = frame_meta->source_id;
                        msg_meta->placeId = frame_meta->source_id;
                        msg_meta->moduleId = 0;
                        msg_meta->sensorStr = g_strdup("sensor-0");
                        msg_meta->type = NVDS_EVENT_ENTRY;
                        generate_event_msg_meta(msg_meta, PGIE_CLASS_ID_PERSON);

                        NvDsUserMeta *user_event_meta =
                                nvds_acquire_user_meta_from_pool(batch_meta);

                        if (user_event_meta) {
                            user_event_meta->user_meta_data = (void *) msg_meta;
                            user_event_meta->base_meta.meta_type = NVDS_EVENT_MSG_META;
                            user_event_meta->base_meta.copy_func =
                                    (NvDsMetaCopyFunc) meta_copy_func;
                            user_event_meta->base_meta.release_func =
                                    (NvDsMetaReleaseFunc) meta_free_func;
                            nvds_add_user_meta_to_frame(frame_meta, user_event_meta);
                        } else {
                            g_print("Error in attaching event meta to buffer\n");
                        }
                    }
                    if (PGIE_CLASS_ID_PERSON == obj_meta->class_id && lane_string.str() =="Exit") {

                        g_print("Person crossed the line\n");
                        g_print(lane_string.str().c_str());
                        NvDsEventMsgMeta *msg_meta =
                                (NvDsEventMsgMeta *) g_malloc0(sizeof(NvDsEventMsgMeta));
                        msg_meta->bbox.top = obj_meta->rect_params.top;
                        msg_meta->bbox.left = obj_meta->rect_params.left;
                        msg_meta->bbox.width = obj_meta->rect_params.width;
                        msg_meta->bbox.height = obj_meta->rect_params.height;
                        msg_meta->frameId = frame_meta->frame_num;
                        msg_meta->trackingId = obj_meta->object_id;
                        msg_meta->confidence = obj_meta->confidence;

                        msg_meta->sensorId = frame_meta->source_id;
                        msg_meta->placeId = frame_meta->source_id;
                        msg_meta->moduleId = 0;
                        msg_meta->sensorStr = g_strdup("sensor-0");
                        msg_meta->type = NVDS_EVENT_EXIT;
                        generate_event_msg_meta(msg_meta, PGIE_CLASS_ID_PERSON);
                        NvDsUserMeta *user_event_meta =
                                nvds_acquire_user_meta_from_pool(batch_meta);

                        if (user_event_meta) {
                            user_event_meta->user_meta_data = (void *) msg_meta;
                            user_event_meta->base_meta.meta_type = NVDS_EVENT_MSG_META;
                            user_event_meta->base_meta.copy_func =
                                    (NvDsMetaCopyFunc) meta_copy_func;
                            user_event_meta->base_meta.release_func =
                                    (NvDsMetaReleaseFunc) meta_free_func;
                            nvds_add_user_meta_to_frame(frame_meta, user_event_meta);
                        } else {
                            g_print("Error in attaching event meta to buffer\n");
                        }
                    }
                }
            }
//            g_print("component id: %d\n", obj_meta->unique_component_id);
//            g_print("class id: %d\n", obj_meta->class_id);
            // list of object_ids detected

            if (obj_meta->unique_component_id == 2) {
                if (obj_meta->rect_params.width < 50.0 || obj_meta->rect_params.height < 50.0) {
                    continue;
                }
                num_rects++;

                char fileFrameNameString[FILE_NAME_SIZE];
                // add object_id to list


                //interate through the list of object_ids
//                for (int i = 0; i < object_ids.size(); i++) {
//                    g_print("object_ids: %d\n", object_ids[i]);
//                }
                // check if object_id parent id  is already in the list of object_ids if yes then continue

//                if (std::find(object_ids.begin(), object_ids.end(), obj_meta->parent->object_id) != object_ids.end()) {
//                    g_print("Object already in the list\n");
//                    continue;
//                }
//                g_print("Pushing object_id: %d\n", obj_meta->parent->object_id);
//                object_ids.push_back(obj_meta->parent->object_id);
                // filename object id and frame number
//                snprintf(fileFrameNameString, FILE_NAME_SIZE, "object_%d_frame_%d.jpg",
//                         obj_meta->object_id, frame_number);
//
//                g_print("Saving fileFrameNameString: %s\n", fileFrameNameString);
//                NvDsObjEncUsrArgs objData = {0};
//                /* To be set by user */
//                // set filename
//                strcpy(objData.fileNameImg, fileFrameNameString);
//                objData.saveImg = save_img;
//                objData.attachUsrMeta = attach_user_meta;
//                /* Set if Image scaling Required */
//                objData.scaleImg = FALSE;
//                objData.scaledWidth = 0;
//                objData.scaledHeight = 0;
//                /* Preset */
//                objData.objNum = num_rects;
//                /* Quality */
//                objData.quality = 80;
//                /*Main Function Call */
//                nvds_obj_enc_process((NvDsObjEncCtxHandle) ctx, &objData, ip_surf, obj_meta, frame_meta);
            }

        }


    }
    nvds_obj_enc_finish((NvDsObjEncCtxHandle) ctx);
    return GST_PAD_PROBE_OK;
}

/* osd_sink_pad_buffer_probe  will extract metadata received on OSD sink pad
 * and update params for drawing rectangle, object information etc. */

extern GstPadProbeReturn
osd_sink_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info,
                          gpointer u_data) {
    GstBuffer *buf = (GstBuffer *) info->data;
    NvDsFrameMeta *frame_meta = NULL;
    NvOSD_TextParams *txt_params = NULL;
    guint vehicle_count = 0;
    guint person_count = 0;
    gboolean is_first_object = TRUE;
    NvDsMetaList *l_frame, *l_obj;

    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    for (l_frame = batch_meta->frame_meta_list; l_frame != NULL;
         l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) (l_frame->data);
        int offset = 0;
        /* To verify  encoded metadata of cropped frames, we iterate through the
        * user metadata of each frame and if a metadata of the type
        * 'NVDS_CROP_IMAGE_META' is found then we write that to a file as
        * implemented below.
        */
        char fileFrameNameString[FILE_NAME_SIZE];
        const char *osd_string = "OSD";

        /* For Demonstration Purposes we are writing metadata to jpeg images of
          * the first 10 frames only.
          * The files generated have an 'OSD' prefix. */

        NvDsUserMetaList *usrMetaList = frame_meta->frame_user_meta_list;
        FILE *file;
        int stream_num = 0;
        while (usrMetaList != NULL) {


            NvDsUserMeta *usrMetaData = (NvDsUserMeta *) usrMetaList->data;
            if (usrMetaData->base_meta.meta_type == NVDS_CROP_IMAGE_META) {
                g_print("got user meta data\n");
                snprintf(fileFrameNameString, FILE_NAME_SIZE, "%s_frame_%d_%d.jpg",
                         osd_string, frame_number, stream_num++);
                NvDsObjEncOutParams *enc_jpeg_image =
                        (NvDsObjEncOutParams *) usrMetaData->user_meta_data;
                /* Write to File */
                file = fopen(fileFrameNameString, "wb");
                fwrite(enc_jpeg_image->outBuffer, sizeof(uint8_t),
                       enc_jpeg_image->outLen, file);
                fclose(file);
                callSendFilePathAsynchronously(fileFrameNameString);


            }
            usrMetaList = usrMetaList->next;
        }

    }
    frame_number++;
    return GST_PAD_PROBE_OK;
}
