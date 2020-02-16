# destination name
NAME=orbits1

# installation prefix
#PREFIX=/data/wa

# the webpath for becomes the installation prefix followed by the program name
WEBPATH=${PREFIX}/${NAME}

CC=emcc
CFLAGS=-std=c++17 -Os
OPTIONS=-s WASM=1 -s USE_WEBGL2=0 -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 -s TOTAL_MEMORY=39321600 -s FORCE_FILESYSTEM=1

DEPS= main.cpp draw.hpp font-data-20.data font-data-20.data.br font.hpp gl.hpp shader_text.hpp vec3.hpp widget_text.hpp points.hpp shell_fullscreen.html

build/$(NAME).html: $(DEPS) 
	mkdir -p build
	$(CC) $(CFLAGS) main.cpp $(OPTIONS) -o build/$(NAME).html --shell-file shell_fullscreen.html
	cp terrain_atlas.png build/

clean:
	rm -rf build

# copy the web files to the web server path and generate gzip and brotli compressed versions
install:
	mkdir -p ${WEBPATH}
	cp build/${NAME}.html ${WEBPATH}/index.html 
	cp build/$(NAME).js build/$(NAME).wasm $(WEBPATH)/
	cd $(WEBPATH); brotli -f index.html -o index.html.br; brotli -f $(NAME).js -o $(NAME).js.br; brotli -f $(NAME).wasm -o $(NAME).wasm.br
	cd $(WEBPATH); gzip -f -k index.html; gzip -f -k $(NAME).js; gzip -f -k $(NAME).wasm
	cp terrain_atlas.png ${WEBPATH}