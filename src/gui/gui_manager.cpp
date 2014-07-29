#include "component.hpp"
#include "gui_manager.hpp"

#include <new>
#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifdef USE_GLES

#include <GLES2/gl2.h>

#endif

#ifdef USE_GL
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#define TILESET_ELEMENT_SIZE 16
#define IMAGE2_SIZE_WIDTH 192
#define IMAGE2_SIZE_HEIGHT 128
#define GLOBAL_SCALE 2
#define IMAGE2_NUM_COMPONENTS 4

void GUIManager::parse_components() {
    //Now generate the needed rendering data
    generate_tex_data();
    generate_vertex_data();
    load_textures();
    init_shaders();
}

void GUIManager::update_components() {
    
}

void GUIManager::mouse_callback_function(MouseInputEvent event) {

    //Just get the end state for the moment
    /*
      std::cout << "X " << event.to.x << " Y " << event.to.y << std::endl;
      if(event.button & MouseState::Button::LEFT) {
      std::cout << "LEFT" << std::endl;
      }
      if(event.button & MouseState::Button::RIGHT) {
      std::cout << "RIGHT" << std::endl;
      }
      if(event.button & MouseState::Button::MIDDLE) {
      std::cout << "MIDDLE" << std::endl;
      }
     
    */
    //Work out which component was clicked
    int mouse_x = event.to.x;
    int mouse_y = event.to.y;

    int curr_x_offset = 0;
    int curr_y_offset = 0;

    //Traverse the component tree
    recurse_components(root, mouse_x, mouse_y, curr_x_offset, curr_y_offset);
}

bool GUIManager::recurse_components(std::shared_ptr<Component> root, int mouse_x, int mouse_y, int curr_x_offset, int curr_y_offset) {
    try{
        //Go through all the children of this component
        for(auto component_pair : root->get_components()) {
            std::shared_ptr<Component> component = component_pair.second;
            
            //Get the component dimensions
            int component_width_pixels = component->get_width_pixels();
            int component_x_offset_pixels = component->get_x_offset_pixels();
            int component_y_offset_pixels = component->get_y_offset_pixels();
            int component_height_pixels = component->get_height_pixels();

            //The x and y offset of this component relative to the origin
            int x_offset = curr_x_offset + component_x_offset_pixels;
            int y_offset = curr_y_offset + component_y_offset_pixels;

            //The offsets which account for the width and height of the component
            int x_right_offset = x_offset + component_width_pixels;
            int y_top_offset = y_offset + component_height_pixels;

            //bounds test
            //Check to see if the click is where this component sits
            if(mouse_x >= x_offset && mouse_x <= x_right_offset && 
               mouse_y >= y_offset && mouse_y <= y_top_offset) {
                //Ok, so this component is in bounds - check if its is clickable or has children

                if(component->is_clickable()) {
                    //Call the click event handler
                    component->call_on_click();
                
                    //The click has been handled
                    return true;
                }

                //It's not clickable, traverse the children
                if(recurse_components(component, mouse_x, mouse_y, x_offset, y_offset))  {
                    //Click has been handled
                    return true;
                }
                
                //Click has not been handled
                //Move onto next component in the tree (Next sibling).
            }
        }
    }
    catch(component_no_children_exception& e) {
        //Catch the exception and return false
        return false;
    }
    return false;
}

GUIManager::GUIManager() : gui_tex_data(nullptr), gui_data(nullptr), tex_buf(nullptr) {

}

GUIManager::~GUIManager() {
    delete []gui_tex_data;
    delete []gui_data;
    delete []tex_buf;
}


void GUIManager::generate_tex_data() {
    
    //delete it if its already allocated
    delete []gui_tex_data; 

    //generate the texture data data
    std::vector<std::pair<GLfloat*, int>> components_data = root->generate_texture_data();

    //calculate data size
    long num_floats = 0;
    for(auto component_texture_data : components_data) {
        num_floats += component_texture_data.second;
    }

    //Create a buffer for the data
    try {
        gui_tex_data  = new GLfloat[sizeof(GLfloat)*num_floats]; 
    }
    catch(std::bad_alloc& ba) {
        std::cerr << "ERROR: bad_alloc caught in GUIManager::generate_tex_data()" << ba.what() << std::endl;
        return;
    }


    int gui_tex_data_offset = 0;

    //Extract the data
    for(auto component_texture_data : components_data) {
        GLfloat* texture_coords = component_texture_data.first;
        size_t texture_coords_size = size_t(component_texture_data.second);

        //copy data into buffer
        std::copy(texture_coords, &texture_coords[texture_coords_size], &gui_tex_data[gui_tex_data_offset]);

        gui_tex_data_offset += component_texture_data.second;
    }

    //Generate the data
    renderable_component.set_texture_coords_data(gui_tex_data, sizeof(GLfloat)*num_floats, false);
} 

void GUIManager::generate_vertex_data() {
    
    //Delete the data if its already allocated
    delete []gui_data;

    //generate the vertex data
    std::vector<std::pair<GLfloat*, int>> components_data = root->generate_vertex_data();

    //calculate data size
    long num_floats = 0;
    for(auto component_vertex_data : components_data) {
        num_floats += component_vertex_data.second;
    }

    //Create a buffer for the data
    try {
        gui_data  = new GLfloat[sizeof(GLfloat)*num_floats]; 
    }
    catch(std::bad_alloc& ba) {
        std::cerr << "ERROR: bad_alloc caught in GUIManager::generate_vertex_data()" << ba.what() << std::endl;
        return;
    }


    int gui_data_offset = 0;
    //Extract the data
    for(auto component_vertex_data : components_data) {
        GLfloat* vertices = component_vertex_data.first;
        size_t vertices_size = size_t(component_vertex_data.second);
        
        //copy data into buffer
        std::copy(vertices, &vertices[vertices_size], &gui_data[gui_data_offset]);

        gui_data_offset += component_vertex_data.second;
    }

    renderable_component.set_vertex_data(gui_data,sizeof(GLfloat)*num_floats, false);
    renderable_component.set_num_vertices_render(GLsizei(num_floats/3));//GL_TRIANGLES being used
}

void GUIManager::load_textures() {


    FILE *tex_file2 = nullptr;
    size_t bytes_read = 0;
    size_t image_sz_2 = IMAGE2_SIZE_WIDTH*IMAGE2_SIZE_HEIGHT*IMAGE2_NUM_COMPONENTS;

    tex_buf = new char[image_sz_2];

    //TODO: use the actual gui texture
    tex_file2 = fopen("../resources/characters_1.raw", "rb");
    if(tex_file2 == nullptr) {
        std::cerr << "ERROR: Couldn't load textures" << std::endl;
    }

    if (tex_file2 && tex_buf) {
        bytes_read = fread(tex_buf, 1, image_sz_2, tex_file2);
        assert(bytes_read == image_sz_2);  // some problem with file?
        fclose(tex_file2);
    }
    //Set the texture data in the rederable component
    renderable_component.set_texture_data(tex_buf, static_cast<int>(image_sz_2), IMAGE2_SIZE_WIDTH, IMAGE2_SIZE_HEIGHT, false);

}
bool GUIManager::init_shaders() {

    Shader* shader = nullptr;
    try {
#ifdef USE_GLES
        shader = new Shader("vert_shader.glesv", "frag_gui_shader.glesf");
#endif
#ifdef USE_GL
        shader = new Shader("vert_shader.glv", "frag_gui_shader.glf");
#endif
    }
    catch (std::exception e) {

        delete shader;
        shader = nullptr;
        LOG(ERROR) << "Failed to create the shader";
        return false;
    }

    //Set the shader
    renderable_component.set_shader(shader);

    return true;
}
