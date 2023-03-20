set(COMPONENT_SRCS "main.c")
set(COMPONENT_ADD_INCLUDE_DIRS "." "../files")

set (COMPONENT_EMBED_TXTFILES
        "../files/network/index.html"
        )


register_component()