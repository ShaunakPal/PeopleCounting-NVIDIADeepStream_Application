//
// Created by kamoliddin on 9/26/23.
//


#ifndef DS_SCH_PEOPLE_COUNT_MULTISTREAM_UTILS_H
#define DS_SCH_PEOPLE_COUNT_MULTISTREAM_UTILS_H



void
cb_newpad(GstElement *decodebin, GstPad *decoder_src_pad, gpointer data);
void
decodebin_child_added(GstChildProxy *child_proxy, GObject *object,
                      gchar *name, gpointer user_data);
GstElement *
create_source_bin(guint index, gchar *uri);

gboolean
bus_call(GstBus *bus, GstMessage *msg, gpointer data);

#endif //DS_SCH_PEOPLE_COUNT_MULTISTREAM_UTILS_H