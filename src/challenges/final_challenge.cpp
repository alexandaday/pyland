#include <glog/logging.h>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream>
#include "final_challenge.hpp"
#include "challenge_helper.hpp"
#include "challenge_data.hpp"
#include "challenge.hpp"
#include "engine.hpp"
#include "map_object.hpp"
#include "object_manager.hpp"
#include "entitythread.hpp"
#include "sprite.hpp"
#include "api.hpp"
#include "object.hpp"
#include "interpreter.hpp"
#include "make_unique.hpp"

// fruit avalible and the number of each in the map
std::vector<std::string> fruit_types = {"bananas","orange","pineapple"};
unsigned int num_of_fruit = 3;

FinalChallenge::FinalChallenge(ChallengeData *challenge_data): Challenge(challenge_data) {

    //creating monkey helper
    int monkey_id = ChallengeHelper::make_sprite(
        this,
        "sprite/monkey",
        "Milo",
        Walkability::BLOCKED,
        "south/still/1"
    );


    //create the main game character
    int player_id = ChallengeHelper::make_sprite(
        this,
        "sprite/1",
        "Ben",
        Walkability::BLOCKED,
        "south/still/1"
    );

    // get the locations of handover point in the map
    glm::ivec2 handover_location = ChallengeHelper::get_location_interaction("handover/1");
    glm::ivec2 handover_pickup = ChallengeHelper::get_location_interaction("pickup/1");
    glm::ivec2 handover_dropoff = ChallengeHelper::get_location_interaction("dropoff/1");


    for (std::string fruit_type : fruit_types) {

        // setup fruit dropoff locations
        glm::ivec2 crate_location = ChallengeHelper::get_location_interaction("crate/"+fruit_type);
        glm::ivec2 dropoff_location = ChallengeHelper::get_location_interaction("dropoff/"+fruit_type);

        // adding fruits as pickupable objects
        for (unsigned int i = 1; i <= num_of_fruit; i++) {
            auto name = fruit_type+"/"+std::to_string(i);
            glm::ivec2 fruit_location = ChallengeHelper::get_location_object(name);

            int fruit_id = ChallengeHelper::make_object(this, name, Walkability::WALKABLE, fruit_type);
            ChallengeHelper::create_pickupable(fruit_location, fruit_location, crate_location, dropoff_location , fruit_id, true);
            ChallengeHelper::create_pickupable(handover_location,handover_pickup,handover_location,handover_dropoff,fruit_id, true);
        }

        // checking if all fruits are in crate
        ChallengeHelper::make_interaction("dropoff/"+fruit_type, [crate_location, fruit_type, this] (int) {

            // notify the user when on crate is full
            if (Engine::get_objects_at(crate_location).size()==num_of_fruit) {
                Engine::print_dialogue ("Game","Well Done, all the "+fruit_type+"s are in the crate");
                return false;
            } else {
                return true;
            }
        });

    }


    // challenge exit
    ChallengeHelper::make_interaction("exit", [this] (int) {

        // function to test of termination critera of challenge has been meet
        bool terminate =
            std::all_of(std::begin(fruit_types), std::end(fruit_types), [&] (std::string fruit_type) {
                glm::ivec2 crate_location = ChallengeHelper::get_location_interaction("crate/"+fruit_type);

                return Engine::get_objects_at(crate_location).size()==num_of_fruit;
            });

        if (terminate) {
            finish();
            LOG(INFO) << "exiting final challenge";
            return false;
        } else {
            // ben put text here
            Engine::print_dialogue("Villager","Hello again adventurer! \n"
                "Try to complete this level and come back to when you think you've finished.\n");
            return true;
        }
    });

    // vector to store object id's of crocodiles
    std::vector<int> croc_ids;

    for (int i = 1; i<=3; i++) {
        std::string croc_num = std::to_string(i);


        LOG(INFO) << "creating croc " << croc_num;
        int croc_id = ChallengeHelper::make_object(
            this,
            "sprite/crocodile/"+croc_num,
            Walkability::BLOCKED,
            "north/still/1"
        );
        croc_ids.push_back(croc_id);

        auto croc = ObjectManager::get_instance().get_object<Object>(croc_id);
        if (!croc) { return; }

        // Register user controled sprite
        // Yes, this is a memory leak. Deal with it.
        auto properties(map->locations.at("Objects/sprite/crocodile/"+croc_num));
        auto *a_thing(new Entity(properties.location, "final_challenge_croc_"+croc_num, croc_id));

        LOG(INFO) << "Registering sprite";
        croc->daemon = std::make_unique<LockableEntityThread>(challenge_data->interpreter->register_entity(*a_thing));
        LOG(INFO) << "Done!";
        croc->daemon->value->halt_soft(EntityThread::Signal::RESTART);
    }

    //Adding cuttable object
    auto vine_name = "vines/cut/1";
    int vine_id = ChallengeHelper::make_object(this, vine_name, Walkability::BLOCKED, "3", true);
    std::shared_ptr<MapObject> vines_object = ObjectManager::get_instance().get_object<MapObject>(vine_id);

    //Adding bridge object
    auto bridge_name = "bridge/1";
    int bridge_id = ChallengeHelper::make_object(this, bridge_name, Walkability::BLOCKED, "4", false); //Made Bridge uncuttable as users can get stuck without a bridge!!!!! :(

    // Game Dialogue

    Engine::print_dialogue("Villager",
                           "Hello adventurer! I'm in need of some help.\n"
                           "You see, I need to collect some fruit for the villagers.\n"
                           "Unfortunately, the bridge is broken and I can't get out into \n"
                           "the jungle to gather the fruit.\n"
    );

    EventManager::get_instance().add_timed_event(GameTime::duration(5.0), [this] (float completion) {
            if(completion == 1.0) {
                Engine::print_dialogue("Villager",
                                       "You can repair the bridge with vines. \n"
                                       "Maybe you could use your friend Milo to help you? \n"
                                       "Try using the cut(direction) API call."
                                       );
            }
            return true;
    });

    ChallengeHelper::make_interaction("fixbridge/1", [bridge_id] (int){
            auto object = ObjectManager::get_instance().get_object<MapObject>(bridge_id);

            if(!object)
                return true;
            object->set_tile(object->frames.get_frame("3"));
            object->set_walkability(Walkability::WALKABLE);

            Engine::print_dialogue ("Villager",
                                    "Excellent! Thanks for fixing that bridge for me.\n"
                                    "I'll tell you what, if you gather the fruit for me, \n"
                                    "I'll give you a reward for your help!\n"
            );


            EventManager::get_instance().add_timed_event(GameTime::duration(5.0), [] (float completion) {
                    if(completion == 1.0) {
                        Engine::print_dialogue("Villager",
                                               "It can be quite a tedious process as the fruit is a "
                                               "long way into the jungle.\n We normally work in pairs "
                                               "when we gather the fruit.\n There's a drop off point "
                                               "on the map where we exchange fruits with each other.\n"
                                               "Why not get Milo to pick up items from that point "
                                               "and run them back to the fruit crates by me?\n"
                                               "That way, you're free to gather more fruit whilst "
                                               "he does that!"
                        );
                    }
                    return true;
                });

            return false;
    });

    //Check to see if the player/monkey and crocs meet at the bridge
    for (int i=1; i<=2; i++) {

        std::string bridge_name = "killplayer/"+std::to_string(i);

        ChallengeHelper::make_interaction(bridge_name, [player_id, monkey_id, croc_ids, this] (int) {
                auto player = ObjectManager::get_instance().get_object<Sprite>(player_id);
                if(!player)
                    return true;

                auto monkey = ObjectManager::get_instance().get_object<Sprite>(monkey_id);
                if(!monkey)
                    return true;

                for(int croc_id : croc_ids) {
                    auto object = ObjectManager::get_instance().get_object<MapObject>(croc_id);

                    if(!object)
                        return true;

                    //If the player and the croc meets, restart the challenge for the player
                    if(object->get_position() == player->get_position() || object->get_position() == monkey->get_position()) {

                        glm::vec2 object_posn = object->get_position();
                        ChallengeHelper::kill_sprite(this, player_id, object_posn, "Villager", "Oh no! The Crocodile got you!" );
                        ChallengeHelper::kill_sprite(this, monkey_id, object_posn, "Villager", "Oh no! The Crocodile got you!" );
                    }
                }

                return true;
            });
    }

}

void FinalChallenge::start() {
}

void FinalChallenge::finish() {
    ChallengeHelper::set_completed_level(3);
    Engine::print_dialogue("Villager",
                           "Fantastic! Thanks for helping me out there. As a token of my gratitude, "
                           "please accept this treasure."
    );
    event_finish.trigger(0);
}
