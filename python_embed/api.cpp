#include <string>
#include <iostream>
#include <sstream>
#include <boost/python.hpp>
#include "api.h"

namespace py = boost::python;

Vec2D::Vec2D(int x, int y): x(x), y(y) {};

Vec2D Vec2D::operator+(Vec2D other) {
    return Vec2D(x + other.x, y + other.y);
}

void Vec2D::operator+=(Vec2D other) {
    x += other.x;
    y += other.y;
}

std::ostream& operator<<(std::ostream& out, Vec2D in) {
    return out << in.to_string();
}

std::string Vec2D::to_string() {
    std::ostringstream stringStream;
    stringStream << "(" << x << " " << y << ")";
    return stringStream.str();
}



Player::Player(Vec2D start, std::string name):
    position(start), script("") {
        this->name = std::string(name);
}

long Player::call_number = 0;

void Player::move(Vec2D by) {
    position += by;
}

void Player::monologue() {
    std::cout << "I am " << name << " and I am standing at " << position << "!" << std::endl;
}

void Player::run_script() {
    ++call_number;
    script(py::ptr(this));
}

void Player::give_script(py::api::object callable) {
    script = callable;
}
