
#pragma once

#include "module.h"
#include <string>
#include <vector>
#include <memory>

namespace orf_n {

class program {
public:
	/* Create a new shaderprogram based on module objects.
	 * A vector of shared pointers. It is kept as long as the program is alive. */
	program( const std::vector<std::shared_ptr<module>> &modules );

	virtual ~program();

	GLuint get_program() const;

	void use() const;

	void un_use() const;

private:
	GLuint m_program;

	void link() const;

};

}
