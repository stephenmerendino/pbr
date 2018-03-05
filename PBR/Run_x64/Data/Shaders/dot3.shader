<shader>
    <shader_program src="Data/HLSL/Lighting/normal_and_spec_map.hlsl" geometry_stage="false"/>
    <!--- <shader_program src="Data/HLSL/Lighting/dot3_lighting.hlsl" geometry_stage="false"/> -->

    <depth write="true" test="less" />
    <raster cull="back" fill="solid" />
    <blend>
        <color src="one" dest="zero" op="add" />
        <alpha src="one" dest="zero" op="add" />
    </blend>
</shader>