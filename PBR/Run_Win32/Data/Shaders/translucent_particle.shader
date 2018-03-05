<shader>
    <shader_program>
        <vertex src="Data/HLSL/mvp.vert" />
        <fragment src="Data/HLSL/nop_texture.frag" />
    </shader_program>

    <depth write="true" test="false" />
    <raster cull="back" fill="solid" />
    <blend>
        <color src="src" dest="inv_src" op="add" />
        <alpha src="one" dest="one" op="add" />
    </blend>
</shader>
