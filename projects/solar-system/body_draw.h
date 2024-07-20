#pragma once
#include "body.h"
#include <framework.h>

struct body_draw
{
	void setup();
	void draw(body_node& body, float radius, const frame::col4& color);

private:
	sg_pipeline pip_planet;
	sg_bindings bind_planet;
};
