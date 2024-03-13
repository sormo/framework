#pragma once

struct sapp_event;

void events_end_frame();
void handle_event(const sapp_event* event);