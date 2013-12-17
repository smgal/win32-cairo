
#include <cairo.h>

extern "C" void cairo_main(void);

namespace win32
{
	void flush(cairo_t* p_cairo);
}

void cairo_main(void)
{
	cairo_surface_t* p_cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 500, 500);
	cairo_t* p_cairo = cairo_create(p_cairo_surface);

	cairo_surface_destroy(p_cairo_surface);

	{
		cairo_new_path(p_cairo);
		cairo_rectangle(p_cairo, 0.0, 0.0, 500.0, 500.0);

		cairo_set_source_rgba(p_cairo, 0.20, 0.20, 0.20, 0.0);
		cairo_fill(p_cairo);

		cairo_new_path(p_cairo);
		cairo_move_to(p_cairo,  68.0,  83.0);
		cairo_line_to(p_cairo, 265.0, 418.0);
		cairo_line_to(p_cairo, 288.0,  62.0);
		cairo_line_to(p_cairo,  38.0, 352.0);
		cairo_line_to(p_cairo, 392.0, 268.0);

		cairo_close_path(p_cairo);

		cairo_set_fill_rule(p_cairo, CAIRO_FILL_RULE_EVEN_ODD);
		cairo_set_source_rgba(p_cairo, 0.2, 0.3, 1.0, 1.0);
		cairo_fill_preserve(p_cairo);

		cairo_set_source_rgba(p_cairo, 0.5, 1.0, 0.75, 1.0);
		cairo_set_line_width(p_cairo, 10.0);
		cairo_stroke(p_cairo);
	}

	{
		cairo_surface_t* p_image = cairo_image_surface_create_from_png("test32.png");

		cairo_set_source_surface(p_cairo, p_image, 0, 0);
		cairo_paint(p_cairo);

		cairo_translate(p_cairo, 160.0, 120.0);
		cairo_rotate(p_cairo, -45.0 * 3.141592 / 180.0);
		cairo_translate(p_cairo, -160.0, -120.0);

		cairo_set_source_surface(p_cairo, p_image, 0, 0);
		cairo_paint_with_alpha(p_cairo, 0.3f);

		cairo_surface_destroy(p_image);
	}

	win32::flush(p_cairo);

	cairo_destroy(p_cairo);
}
