#version 430
#extension GL_ARB_compute_shader :                  enable
#extension GL_ARB_shader_storage_buffer_object :     enable

struct v2
{
    float x;
    float y;
};


uniform int updateRate;
uniform float pointSize;

uniform ivec2 boxSize;




layout (std430, binding=1) buffer d1{
    v2 pos[];
};


layout (std430, binding=2) buffer d2{
    v2 vel[];
};

layout( local_size_x = 1,  local_size_y = 1, local_size_z = 1 )   in;


void main()
{
    uint gid = gl_GlobalInvocationID.x;
    vec2 p = vec2(pos[gid].x,pos[gid].y);
    vec2 v = vec2(vel[gid].x,vel[gid].y);
    if(p.x>boxSize.x-pointSize){
        p.x = boxSize.x-pointSize;
        v.x = -v.x;
    }
    if(p.y>boxSize.y-pointSize){
        p.y = boxSize.y-pointSize;
        v.y = -v.y;
    }
    if(p.x<pointSize){
        p.x = pointSize;
        v.x = -v.x;
    }
    if(p.y<pointSize){
        p.y = pointSize;
        v.y = -v.y;
    }

    p +=v/updateRate;
    

    for(int i=0; i<gl_NumWorkGroups.x;i++){
        if(i == gid)
            continue;
        vec2 p2 = vec2(pos[i].x,pos[i].y);
        vec2 v2 = vec2(vel[i].x,vel[i].y);
        float dst = distance(p,p2);
        vec2 center = (p2 + p)/2.0;
        vec2 diff = normalize(p2 - p);
        if(dst <= pointSize*2)
        {
            p = center - diff*pointSize*1.01;
    
            v = -diff *length(v);
     
            
            //pos[i].x = p2.x;
            //pos[i].y = p2.y;
            //vel[i].x = v2.x;
            //vel[i].y = v2.y;
        }
    }

    barrier();

    pos[gid].x = p.x;
    pos[gid].y = p.y;

    vel[gid].x = v.x;
    vel[gid].y = v.y;
}