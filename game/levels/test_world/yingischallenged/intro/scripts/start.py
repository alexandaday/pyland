import sys
import os

sys.path.insert(1, "../game/components")
from scoped_interpreter import ScopedInterpreter

engine.clear_scripter()
engine.insert_to_scripter("howdy")
engine.insert_to_scripter("poster")

print(engine.get_script)



camera.focus()
c1 = (50, 50, 50)
c2 = (110, 110, 110)
engine.set_ui_colours(c1, c2)
engine.play_music("calm")
engine.add_dialogue(engine.get_dialogue("intro_coming_now"))
engine.add_dialogue(engine.get_dialogue("intro_im_monty_the_snake"))
engine.add_dialogue(engine.get_dialogue("intro_monty_doesnt_know_name"))
engine.add_dialogue(engine.get_dialogue("intro_monty_text_editor"))
engine.add_dialogue(engine.get_dialogue("intro_big_white_box"))
engine.add_dialogue(engine.get_dialogue("intro_monty_hello_player", {"player_name": "Alex"}))
engine.add_dialogue(engine.get_dialogue("intro_wrote_first_program", {"player_name": "Alex"}))
engine.open_dialogue_box() #Give callback here?

#engine.change_map("test_world/yingischallenged/main")

#engine.print_terminal(player_one.get_position(), False)
#camera.move_to_position(position, time)
camera.move_west(lambda: camera.move_west(lambda: camera.move_west(lambda: camera.move_east(lambda: camera.move_east()))))

#engine.print_terminal("Switching level")
#camera.wait(5.0, lambda: engine.change_map("test_world/yingischallenged/main_2"))

def get_name(name):
    engine.print_terminal(name)

imbued_locals_name = {"print" : get_name}

#camera.wait(2, lambda: engine.print_terminal("h"))

