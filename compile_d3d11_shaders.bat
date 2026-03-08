@echo off

if not exist data\shaders\compiled mkdir data\shaders\compiled

fxc /nologo /I "data/shaders" /T vs_5_0 /E vertex_main /Fo data/shaders/compiled/sprite.vertex.fxc data/shaders/sprite.hlsl
fxc /nologo /I "data/shaders" /T ps_5_0 /E pixel_main /Fo data/shaders/compiled/sprite.pixel.fxc data/shaders/sprite.hlsl

fxc /nologo /I "data/shaders" /T vs_5_0 /E vertex_main /Fo data/shaders/compiled/resolve.vertex.fxc data/shaders/resolve.hlsl
fxc /nologo /I "data/shaders" /T ps_5_0 /E pixel_main /Fo data/shaders/compiled/resolve.pixel.fxc data/shaders/resolve.hlsl
