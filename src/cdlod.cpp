
#include "base/logbook.h"
#include "renderer/renderer.h"
#include <iostream>

int main(void) {
	orf_n::logbook::set_log_filename( "orfnlog.log" );
	orf_n::logbook::log_msg( "Program started." );
	try {
		orf_n::renderer* r = new orf_n::renderer(true);
		r->setupRenderer();
		r->setup();
		r->render();
		r->cleanup();
		r->cleanupRenderer();
		delete r;
	} catch( std::runtime_error &e ) {
		std::cerr << e.what() << std::endl;
	}
	orf_n::logbook::log_msg( "Program ending normally." );
	return EXIT_SUCCESS;
}
