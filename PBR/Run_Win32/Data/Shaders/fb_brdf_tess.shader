<shader>
    <shader_program>
        <vertex src="Data/HLSL/mvp.vert" />
		<hull src="Data/HLSL/tri_tess.hull"/>
		<domain src="Data/HLSL/tri_tess.domain"/>
        <fragment src="Data/HLSL/Lighting/fb_brdf.frag" />
    </shader_program>

    <depth write="true" test="less" />
    <raster cull="back" fill="solid" />
    <blend>
        <color src="one" dest="zero" op="add" />
        <alpha src="one" dest="zero" op="add" />
    </blend>
</shader>