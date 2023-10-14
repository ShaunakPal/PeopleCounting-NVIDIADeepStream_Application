//
// Created by kamoliddin on 9/23/23.
//

#ifndef DS_SCH_PEOPLE_COUNT_PAD_PROBE_UTILS_H
#define DS_SCH_PEOPLE_COUNT_PAD_PROBE_UTILS_H
#include "gstnvdsmeta.h"
#include "nvds_analytics_meta.h"
#include "nvdsanalytics_meta_utils.h"
#include "nvbufsurface.h"
#include "nvds_obj_encode.h"
extern  GstPadProbeReturn
nvdsanalytics_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer ctx);

extern GstPadProbeReturn
osd_sink_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);

#endif //DS_SCH_PEOPLE_COUNT_PAD_PROBE_UTILS_H

