<shader>
    <shader_program src="Data/HLSL/Lighting/write_omni_depth.hlsl" geometry_stage="false"/>

    <depth write="true" test="less" />
    <raster cull="back" fill="solid" />
    <blend>
        <color src="one" dest="zero" op="add" />
        <alpha src="one" dest="zero" op="add" />
    </blend>
</shader>