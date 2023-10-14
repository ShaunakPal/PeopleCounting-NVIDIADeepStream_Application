//
// Created by kamoliddin on 9/26/23.
//

#include "nvdsanalytics_meta_utils.h"
#include <gst/gst.h>
#include <glib.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include "gstnvdsmeta.h"
#include "nvds_analytics_meta.h"
#include "analytics.h"

/* custom_parse_nvdsanalytics_meta_data
 * and extract nvanalytics metadata */
extern  void analytics_custom_parse_nvdsanalytics_meta_data (NvDsMetaList *l_user, AnalyticsUserMeta *data)
{
    std::stringstream out_string;
    NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;
    /* convert to  metadata */
    NvDsAnalyticsFrameMeta *meta =
            (NvDsAnalyticsFrameMeta *) user_meta->user_meta_data;
    /* Fill the data for entry, exit,occupancy */
    data->lcc_cnt_entry = 0;
    data->lcc_cnt_exit = 0;
    data->lccum_cnt = 0;
    data->lcc_cnt_entry = meta->objLCCumCnt["Entry"];
    data->lcc_cnt_exit = meta->objLCCumCnt["Exit"];

    if (meta->objLCCumCnt["Entry"]> meta->objLCCumCnt["Exit"])
        data->lccum_cnt = meta->objLCCumCnt["Entry"] - meta->objLCCumCnt["Exit"];
}
