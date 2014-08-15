#include <glog/logging.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api.hpp"
#include "challenge.hpp"
#include "challenge_helper.hpp"
#include "event_manager.hpp"
#include "filters.hpp"
#include "long_walk_challenge.hpp"
#include "notification_bar.hpp"
#include "engine.hpp"
#include "map_viewer.hpp"
#include "object_manager.hpp"
#include "map_object.hpp"
#include "walkability.hpp"
#include "sprite.hpp"

#include <iostream>
LongWalkChallenge::LongWalkChallenge(ChallengeData *challenge_data): Challenge(challenge_data) {
    //Test chest
    int chest_id(make_map_object(
        glm::ivec2(10, 15),
        "test chest",
        Walkability::BLOCKED,
        52,
        "../resources/basictiles_2_64.png"
    ));

    ChallengeHelper::create_pickupable(
        glm::ivec2(10, 15),
        glm::ivec2(10, 14),
        chest_id
    );

    // Ignore references due to non-interaction
    ChallengeHelper::make_object(this, "treasure/path/medium/chest", Walkability::BLOCKED);
    ChallengeHelper::make_object(this, "treasure/path/long/chest",   Walkability::BLOCKED);

    // Testing lawn
    ChallengeHelper::make_interactions("grass/path/short",
                                       std::back_inserter(grass_path_short_callbacks),
                                       [chest_id] (int who) {
        auto sprite(ObjectManager::get_instance().get_object<Sprite>(who));
        if (!sprite) { return true; }

        if (sprite->is_in_inventory(chest_id)) {
            Engine::print_dialogue("Grass", "You're mowing, keep on going");
            Engine::change_tile(sprite->get_position(), 4, "lawn_mown");
            return false;
        }
        else {
            Engine::print_dialogue("Grass", "Don't forget the lawn mower");
            return true;
        }
    });

    // Set up blocking walls
    ChallengeHelper::make_objects(this, "wall/path/medium", Walkability::BLOCKED, std::back_inserter(wall_path_medium_objects));
    ChallengeHelper::make_objects(this, "wall/path/long",   Walkability::BLOCKED, std::back_inserter(wall_path_long_objects));


    // Set up notifications about walls
    ChallengeHelper::make_interactions("blocked_alert/path/medium",
                                       std::back_inserter(blocked_alert_path_medium_callbacks),
                                       [this] (int) {
        Engine::print_dialogue("Tom", "Get the treasure and press \"e\" to view it!\n");

        ChallengeHelper::unregister_all(&blocked_alert_path_medium_callbacks);
        return false;
    });

    ChallengeHelper::make_interactions("blocked_alert/path/long",
                                       std::back_inserter(blocked_alert_path_long_callbacks),
                                       [this] (int) {
        Engine::print_dialogue("Tom", "Hey... you missed the treasure!\n");

        ChallengeHelper::unregister_all(&blocked_alert_path_long_callbacks);
        return false;
    });

    // Set up treasure chest triggers
    ChallengeHelper::make_interaction("treasure/path/medium/carpet", [this] (int) {
        Engine::print_dialogue ("Tom",
            "Who knew treasure was so close?\n"
            "\n"
            "To open it up and see what's inside, press \"e\".\n"
        );

        editor_lifeline = this->challenge_data->input_manager->register_keyboard_handler(filter(
            {ANY_OF({KEY_HELD}), KEY({"E"})},
            [this] (KeyboardInputEvent) {
                std::string id = std::to_string(Engine::get_map_viewer()->get_map_focus_object());
                std::string filename = "John_" + id + ".py";
                Engine::open_editor(filename);
                for (auto wall_object_id : wall_path_medium_objects) {
                    auto wall_object(ObjectManager::get_instance().get_object<MapObject>(wall_object_id));

                    wall_object->set_sheet_id(119);
                    wall_object->set_walkability(Walkability::WALKABLE);
                }
            }
        ));

        return false;
    });

    ChallengeHelper::make_interaction("treasure/path/long/carpet", [this] (int) {
        Engine::print_dialogue("Game",
            "Nice, more treasure!\n"
            "I doubt the next bit is going to be short enough to just walk...\n"
            "You should try to program it.\n"
        );

        for (auto wall_object_id : wall_path_long_objects) {
            auto wall_object(ObjectManager::get_instance().get_object<MapObject>(wall_object_id));

            wall_object->set_sheet_id(119);
            wall_object->set_walkability(Walkability::WALKABLE);
        }
        return false;
    });

    // Some more verbal interaction
    ChallengeHelper::make_interactions("room/first/exit",
                                       std::back_inserter(room_first_exit_callbacks),
                                       [this] (int) {
        Engine::print_dialogue("Tom",
            "Well, hello!\n"
            "I heard there was treasure to the right...\n"
            "You should look for it! It's probably really close!\n"
        );

        ChallengeHelper::unregister_all(&room_first_exit_callbacks);
        return false;
    });


    ChallengeHelper::make_interactions("end",
                                       std::back_inserter(end_callbacks),
                                       [this] (int) {
        // TODO: FINISH SOMEHOW!!
        Engine::print_dialogue("Game", "You Win, Well Done !");
        return true;
    });
}

void LongWalkChallenge::start() {
    Engine::print_dialogue ( "Tom",
        "Welcome to Level 1. The aim of this level to reach the green markers. You win when you reach the last one.\n"
        "My name is Tom and I am here to help you learn to move around \n"
        "You can move your character using arrow keys. Take a walk and look for the first green marker\n"
    );
}

void LongWalkChallenge::finish() {
    // TODO: Somehow finish challenge...
}
