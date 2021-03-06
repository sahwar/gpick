/*
 * Copyright (c) 2009-2016, Albertas Vyšniauskas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of the software author nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Chain.h"

using namespace std;

namespace transformation {

Chain::Chain()
{
	enabled = true;
}

void Chain::apply(const Color *input, Color *output)
{
	if (!enabled) {
		color_copy(input, output);
		return;
	}
	Color tmp[2];
	Color *tmp_p[3];

	color_copy(input, &tmp[0]);

	tmp_p[0] = &tmp[0];
	tmp_p[1] = &tmp[1];
	for (TransformationList::iterator i = transformation_chain.begin(); i != transformation_chain.end(); i++){

		(*i)->apply(tmp_p[0], tmp_p[1]);

		tmp_p[2] = tmp_p[0];
		tmp_p[0] = tmp_p[1];
		tmp_p[1] = tmp_p[2];
	}

	color_copy(tmp_p[0], output);
}

void Chain::add(boost::shared_ptr<Transformation> transformation)
{
	transformation_chain.push_back(transformation);
}

void Chain::remove(const Transformation *transformation)
{
	for (TransformationList::iterator i = transformation_chain.begin(); i != transformation_chain.end(); i++){
		if ((*i).get() == transformation){
			transformation_chain.erase(i);
			return;
		}
	}
}

void Chain::clear()
{
	transformation_chain.clear();
}

Chain::TransformationList& Chain::getAll()
{
	return transformation_chain;
}

void Chain::setEnabled(bool enabled_)
{
	enabled = enabled_;
}

}
