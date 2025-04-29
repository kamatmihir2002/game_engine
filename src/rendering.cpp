#include <systems.h>
#include <scene.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

systems::rendering* current_rendering_system;

unsigned int compileShaders(char** vertex, int* vlen, char** fragment, int* flen) {

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 2, vertex, NULL);
    glCompileShader(vs);
    int  success;
    char infoLog_vs[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vs, 512, NULL, infoLog_vs);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
        printf("%s\n", infoLog_vs);
        printf("\n");
    }

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fs, 2, fragment, NULL);
    glCompileShader(fs);
    char infoLog_fs[512];
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fs, 512, NULL, infoLog_fs);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
        printf("%s\n", infoLog_fs);
        printf("\n");
    }

    unsigned int s = glCreateProgram();
    glAttachShader(s, vs);
    glAttachShader(s, fs);
    glLinkProgram(s);

    return s;
}

systems::rendering* systems::create_rendering_system() {
    systems::rendering* render = (systems::rendering*)malloc(sizeof(systems::rendering));
    scene::resource* resources = scene::get_current_resources();
    systems::transform* tsys = systems::get_current_transform_system();
    int numres = (int)scene::get_current_num_resources();
    render->num_objects = numres;
    render->shader_objects = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    int cam_id = 0;
    for (int i = 0; i < numres; i++) {
        if (resources[i].is_camera) {
            cam_id = i;
            break;
        }
    }
    render->cam_id = cam_id;
    render->cam.position = glm::vec3(tsys->T[cam_id][3]);
    render->cam.look_direction = glm::vec3(0.0, 0.0, 1.0);
    render->cam.up = glm::vec3(0.0, 1.0, 0.0);
    render->cam.view = glm::lookAt(render->cam.position, render->cam.position + render->cam.look_direction, render->cam.up);
    render->cam.proj = glm::perspective<double>((double)glm::radians(45.0), (float)(1920.0f / 1080.0f), 0.1f, 100.0f); 
    render->vaos = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->ibos = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->vbos = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->numinds = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->rendering_enabled = (unsigned char*)malloc(sizeof(unsigned char) * numres);
    render->transform_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->rotscale_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->view_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->proj_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->lightdir_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->diffuse_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->specular_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->shininess_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->campos_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->tex = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->buffers_bound = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->attribs = (ui5*)malloc(sizeof(ui5) * numres);
    render->attribLen = (int*)malloc(sizeof(int) * numres);
    render->lightpos = (glm::vec3*)malloc(sizeof(glm::vec3) * numres);
    render->diffuse = (glm::vec3*)malloc(sizeof(glm::vec3) * numres);
    render->specular = (glm::vec3*)malloc(sizeof(glm::vec3) * numres);
    render->shininess = (float*)malloc(sizeof(float) * numres);
    render->lsm_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->albedo_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);
    render->shadow_map_uniform = (unsigned int*)malloc(sizeof(unsigned int) * numres);

    char* vs_preamble = "#version 330 core\n"\
    "layout (location = 0) in vec3 in_position;\n"\
    "layout (location = 1) in vec3 in_normal;\n"\
    "layout (location = 2) in vec2 in_texture_uvs;\n"\
    "out vec4 object_transformed_position;\n"\
    "out vec4 object_light_space_position;\n"\
    "out vec3 object_normal;\n"\
    "out vec2 object_texture_uvs;\n"\
    "uniform mat4 light_space_matrix;\n"\
    "uniform mat4 model_matrix;\n"\
    "uniform mat4 view_matrix;\n"\
    "uniform mat4 projection_matrix;\n"\
    "uniform mat4 rotation_matrix;\0";

    char* fs_preamble = "#version 330 core\n"\
    "out vec4 FragColor;\n"\
    "in vec3 object_normal;\n"\
    "in vec4 object_transformed_position;\n"\
    "in vec4 object_light_space_position;\n"\
    "in vec2 object_texture_uvs;\n"\
    "uniform sampler2D shadow_map;\n"\
    "uniform vec3 light_direction;\n"\
    "uniform vec3 camera_position;\n"\
    "uniform vec3 material_diffuse;\n"\
    "uniform vec3 material_specular;\n"\
    "uniform float material_shininess;\n"\
    "uniform sampler2D material_albedo;\0";

    float verts[24] = {
        -1.0f, -1.0f, 0.0, 1.0,
        1.0f, -1.0f, 1.0, 1.0,
        -1.0f,  1.0f, 0.0, 0.0,
        -1.0f,  1.0f, 0.0, 0.0,
        1.0f, -1.0f, 1.0, 1.0,
        1.0f,  1.0f, 1.0, 0.0
    };
    // glGenBuffers(1, &render->pps_vbo);
    // glBindBuffer(GL_ARRAY_BUFFER, render->pps_vbo);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, verts, GL_STATIC_DRAW);

    char* vs[2] = {
        "#version 330 core\n"\
        "layout (location = 0) in vec4 pos_uv;\n"\
        "out vec2 uv_position;\n"\
        "void main()\n"\
        "{\n"\
        "   uv_position = vec2(pos_uv.z, pos_uv.w);\n"\
        "    gl_Position = vec4(pos_uv.x, pos_uv.y, 1.0);\n"\
        "}\0",
        "\0"
    };

    for (int i = 0; i < numres; i++) {
        
        if (resources[i].hasmodel) {
            
            render->numinds[i] = resources[i].numinds;
            resources[i].v_shader_str[resources[i].v_shader_str_len] = 0;
            // printf(":ST%s\n:ED", resources[i].v_shader_str);
            char* vs[2] = {vs_preamble, resources[i].v_shader_str};
            int vslen[2] = {426, resources[i].v_shader_str_len};

            // printf(":ST%s\n:ED", resources[i].f_shader_str);
            resources[i].f_shader_str[resources[i].f_shader_str_len] = 0;
            char* fs[2] = {fs_preamble, resources[i].f_shader_str};
            int fslen[2] = {385, resources[i].v_shader_str_len};

            render->shader_objects[i] = compileShaders(
                    vs, vslen, fs, fslen);
        
            render->rendering_enabled[i] = 1;
            
            render->lsm_uniform[i] = glGetUniformLocation(render->shader_objects[i], "light_space_matrix");
            render->shadow_map_uniform[i] = glGetUniformLocation(render->shader_objects[i], "shadow_map");
            render->albedo_uniform[i] = glGetUniformLocation(render->shader_objects[i], "material_albedo");

            render->transform_uniform[i] =  glGetUniformLocation(render->shader_objects[i], "model_matrix");
            
            render->rotscale_uniform[i] =   glGetUniformLocation(render->shader_objects[i], "rotation_matrix");
            render->view_uniform[i] =       glGetUniformLocation(render->shader_objects[i], "view_matrix");
            render->proj_uniform[i] =       glGetUniformLocation(render->shader_objects[i], "projection_matrix");

            render->lightdir_uniform[i] =   glGetUniformLocation(render->shader_objects[i], "light_direction");
            render->campos_uniform[i] =     glGetUniformLocation(render->shader_objects[i], "camera_position");

            render->diffuse_uniform[i] =    glGetUniformLocation(render->shader_objects[i], "material_diffuse");
            render->specular_uniform[i] =   glGetUniformLocation(render->shader_objects[i], "material_specular");
            render->shininess_uniform[i] =  glGetUniformLocation(render->shader_objects[i], "material_shininess");
            
            render->lightpos[i] = render->cam.position;
            render->diffuse[i] = glm::vec3(1.0, 1.0, 1.0);
            render->specular[i] = glm::vec3(1.0, 1.0, 1.0);
            render->shininess[i] = 1.0;
            glGenVertexArrays(1, &render->vaos[i]);

            glGenBuffers(1, &render->vbos[i]);
            glGenBuffers(1, &render->ibos[i]);
            glBindVertexArray(render->vaos[i]);
            glBindBuffer(GL_ARRAY_BUFFER, render->vbos[i]);
            glBufferData(GL_ARRAY_BUFFER, resources[i].full_buffer_len * sizeof(float), resources[i].buffer, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            if (resources[i].hastex) {
                glEnable(GL_TEXTURE_2D);
                glGenTextures(1, &render->tex[i]);
                glBindTexture(GL_TEXTURE_2D, render->tex[i]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resources[i].texw, resources[i].texh, 0, GL_RGBA, GL_UNSIGNED_BYTE, resources[i].texture_data);
            }

            render->attribs[i][0] = 0;
            render->attribs[i][1] = 1;
            render->attribs[i][2] = 2;
            render->attribLen[i] = 3;
                
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render->ibos[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, resources[i].numinds * sizeof(unsigned int), resources[i].indices, GL_STATIC_DRAW);
            glBindVertexArray(0);
            render->buffers_bound[i] = 0;
            render->rendering_enabled[i] = 1;

        }
        else {
            render->rendering_enabled[i] = 0;
        }
    }
    return render;
}

void systems::make_current_rendering_system(systems::rendering* sys) {
    current_rendering_system = sys;
}

void systems::init_rendering_system() {

}

systems::rendering* systems::get_current_rendering_system() {
    return (systems::rendering*)current_rendering_system;
}

systems::lighting* systems::create_lighting_system() {
    systems::lighting* lighting = (systems::lighting*) malloc(sizeof(systems::lighting));
    lighting->num_lights = 0;

    scene::resource* res = scene::get_current_resources();
    int n = scene::get_current_num_resources();

    systems::transform* tsys = systems::get_current_transform_system();

    for (int i = 0; i < n; i++) {
        if (res[i].is_light) {
            int top = lighting->num_lights;
            lighting->transform_id[top] = i;

            lighting->props[top].position = glm::vec3(tsys->T[i][3]);
            lighting->transformed_position[top]  = glm::vec3(tsys->T[i][3]);

            lighting->props[top].look_direction = glm::vec3(1.0, 0.0, 0.0);
            lighting->transformed_look_direction[top] = - glm::vec3(tsys->RS[i] * glm::vec4(lighting->props[top].look_direction, 1.0));

            lighting->props[top].up = glm::vec3(0.0, 1.0, 0.0);
            lighting->transformed_up_direction[top] = glm::vec3(tsys->RS[i] * glm::vec4(lighting->props[top].up, 0.0));

            lighting->props[top].proj = glm::ortho(-16.0f, 16.0f, -16.0f, 16.0f, 1.0f, 32.0f);
            lighting->props[top].view = glm::lookAt(lighting->transformed_position[top], lighting->transformed_position[top] + lighting->transformed_look_direction[top], lighting->transformed_up_direction[top]);

            lighting->num_lights++;
        }

        if (lighting->num_lights >= 8)
            break;
    }

    glGenFramebuffers(1, &lighting->depth_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, lighting->depth_fbo);
    printf("fbo: %lu\n", lighting->depth_fbo);

    glGenTextures(1, &lighting->depth_map);
    glBindTexture(GL_TEXTURE_2D, lighting->depth_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  


    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, lighting->depth_map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    char* v_depth_shader[2] = { "\0", 
        "#version 330 core\n"\
        "layout (location = 0) in vec3 in_position;\n"\
        "uniform mat4 light_space;\n"\
        "uniform mat4 model_matrix;\n"\
        "void main()\n"\
        "{\n"\
        "    gl_Position = light_space * model_matrix * vec4(in_position, 1.0);\n"\
        "}\0"
    };
    
    int vlen[2] = {0, 195};

    char* f_depth_shader[2] = { "\0", 
        "#version 330 core\n"\
        "void main()\n"\
        "{\n"\
        "//gl_FragDepth = gl_FragCoord.z;\n"\
        "}\0"
    };
    
    int flen[2] = {0, 35};
    lighting->depth_shader = compileShaders(v_depth_shader, vlen, f_depth_shader, flen);
    lighting->transform_uniform =  glGetUniformLocation(lighting->depth_shader, "model_matrix");
    lighting->lsm_uniform =  glGetUniformLocation(lighting->depth_shader, "light_space");

    return lighting;
}

systems::lighting* current_lighting_system;

void systems::make_current_lighting_system(systems::lighting* light) {
    current_lighting_system = light;
}

void systems::init_lighting_system() {

}

systems::lighting* systems::get_current_lighting_system() {
    return current_lighting_system;
}

void systems::update_lighting_system() {

    glViewport(0, 0, 1024, 1024);

    systems::lighting* light = systems::get_current_lighting_system();
    systems::transform* tsform = systems::get_current_transform_system();
    
    for (int i = 0; i < light->num_lights; i++) {
        glm::mat4 T = tsform->T[light->transform_id[i]];
        glm::mat4 R = tsform->RS[light->transform_id[i]];
        
        glm::vec4 K = glm::vec4(light->props[i].look_direction, 0.0);
        glm::vec4 U = glm::vec4(light->props[i].up, 0.0);
        
        light->transformed_position[i] = glm::vec3(T[3]);
        light->transformed_look_direction[i] = -glm::vec3(R * K);
        light->transformed_up_direction[i] = glm::vec3(R * U);
        light->props[i].view = glm::lookAt(light->transformed_position[i], light->transformed_position[i] + light->transformed_look_direction[i], light->transformed_up_direction[i]);   
        light->lsm[i] = light->props[i].proj * light->props[i].view;
    }

    systems::rendering* render = systems::get_current_rendering_system();
    
    
    glBindFramebuffer(GL_FRAMEBUFFER, light->depth_fbo);
    glEnable(GL_DEPTH_TEST);  
    glEnable(GL_CULL_FACE);  
    glUseProgram(light->depth_shader);
    glClear(GL_DEPTH_BUFFER_BIT);
    // glBindFramebuffer(GL_FRAMEBUFFER, light->depth_fbo);
    
    glCullFace(GL_FRONT);
    // glBindFramebuffer(GL_FRAMEBUFFER, light->depth_fbo);

    for (int l = 0; l < light->num_lights; l++) {
    
        for (int i = 0; i < render->num_objects; i++) {
            
            if (render->rendering_enabled[i]) {            
                    
                    glUniformMatrix4fv(light->transform_uniform, 1, GL_FALSE, glm::value_ptr(tsform->T[i]));
                    
                    glUniformMatrix4fv(light->lsm_uniform, 1, GL_FALSE, glm::value_ptr(light->lsm[l]));
                    
                    glBindVertexArray(render->vaos[i]);
                    
                    glDrawElements(GL_TRIANGLES, render->numinds[i], GL_UNSIGNED_INT, 0);
                    
                
            }
        }
        
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_CULL_FACE);
}

void systems::prepare_rendering_system(size_t sizex, size_t sizey) {
    glViewport(0, 0, sizex, sizey);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

void systems::update_rendering_system() {
    systems::lighting* light = systems::get_current_lighting_system();
    systems::rendering* render = systems::get_current_rendering_system();
    systems::transform* tsform = systems::get_current_transform_system();

    render->cam.position = glm::vec3(tsform->T[render->cam_id][3]);
    glm::vec3 t_lookdir = glm::vec3(tsform->RS[render->cam_id] * glm::vec4(render->cam.look_direction, 0.0));
    glm::vec3 t_updir = glm::vec3(tsform->RS[render->cam_id] * glm::vec4(render->cam.up, 0.0));
    render->cam.view = glm::lookAt(render->cam.position, render->cam.position + t_lookdir, t_updir);
    
    for (int l = 0; l < light->num_lights; l++) {
    
        for (int i = 0; i < render->num_objects; i++) {
            
            if (render->rendering_enabled[i]) {            
                    GLint prog = 0;
                    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
                    
                    if (prog != render->shader_objects[i]) {
                        glUseProgram(render->shader_objects[i]);
                    }
                    printf("%d\n", glGetError());
                    glUniformMatrix4fv(render->transform_uniform[i], 1, GL_FALSE, glm::value_ptr(tsform->T[i]));
                    glUniformMatrix4fv(render->rotscale_uniform[i], 1, GL_FALSE, glm::value_ptr(tsform->RS[i]));
                    glUniformMatrix4fv(render->view_uniform[i], 1, GL_FALSE, glm::value_ptr(render->cam.view));
                    glUniformMatrix4fv(render->proj_uniform[i], 1, GL_FALSE, glm::value_ptr(render->cam.proj));
                    
                    glUniform3fv(render->lightdir_uniform[i], 1, glm::value_ptr(light->transformed_look_direction[l]));
                    glUniform3fv(render->campos_uniform[i], 1, glm::value_ptr(render->cam.position));
                    glUniform3fv(render->diffuse_uniform[i], 1, glm::value_ptr(render->diffuse[i]));
                    glUniform3fv(render->specular_uniform[i], 1, glm::value_ptr(render->specular[i]));
                    
                    glUniform1f(render->shininess_uniform[i], render->shininess[i]);
                    glUniformMatrix4fv(render->lsm_uniform[i], 1, GL_FALSE, glm::value_ptr(light->lsm[l]));
                    glEnable(GL_TEXTURE_2D);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, render->tex[i]);
                    glUniform1i((GLint)render->albedo_uniform[i], 0);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, light->depth_map);
                    glUniform1i((GLint)render->shadow_map_uniform[i], 1);
                    
                    glBindVertexArray(render->vaos[i]);
                    
                    glDrawElements(GL_TRIANGLES, render->numinds[i], GL_UNSIGNED_INT, 0);
                
                    render->buffers_bound[i] = 1;
                
            }
        }
        
    }
}
