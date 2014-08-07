#include "gui_text.hpp"

#include <memory>

GUIText::GUIText() {
    text_data = std::make_shared<GUITextData>();
}

GUIText::~GUIText() {

}

std::vector<std::pair<GLfloat*, int>> GUIText::generate_vertex_data() {
    std::vector<std::pair<GLfloat*, int>> empty_data;
    return empty_data;
}

std::vector<std::pair<GLfloat*, int>> GUIText::generate_texture_data() {
    std::vector<std::pair<GLfloat*, int>> empty_data;
    return empty_data;    
}

std::vector<std::shared_ptr<GUITextData>> GUIText::generate_text_data() {
    std::vector<std::shared_ptr<GUITextData>> text_data_vec;    
    text_data_vec.push_back(text_data);
    return text_data_vec;
}

std::shared_ptr<GUITextData> GUIText::get_gui_text() {
    return text_data;
}

std::shared_ptr<Text> GUIText::get_text() {
    return text_data->get_text();
}

void GUIText::set_text(std::shared_ptr<Text> _text) {
    text_data->set_text(_text);
}

