#Python modules
import operator
import os
import ctypes #for sending exceptions to python threads!!!

#Custom modules
import scriptrunner


"""
In Python comments,
could define some standard which the C++ code can use to determine things about it handles 
the python code
"""

"""
eg. We could be able to write:
"""
#__ import_object characters/Character
"""
The game code, at runtime, could recognise the "#__"
and replace it with:
"""
import sys
sys.path.insert(1, os.path.dirname(os.path.realpath(__file__)) + '/../../characters')
from character import Character
"""
As the Character is in the characters folder.
"""


""" A brief description of the class will go here.
The auto-generated comment produced by the script should also mention where to get a list of the api
and which built-in variables exist already.
"""
class Player(Character):

    __running_script = False
    __thread_id = 0
    
    def initialise(self):
        """ An initialiser function.
        
        This function is called once all the neccesary set-up of the object has completed
        run when the object is created in-engine
        """
        engine = self.get_engine()

        #register input callbacks to make character playable
        #register callbacks for running player scripts
        engine.register_input_callback(engine.INPUT_RUN, self.run_script)
        engine.register_input_callback(engine.INPUT_HALT, self.halt_script)
        
        #register callbacks for character movement
        engine.register_input_callback(engine.INPUT_UP, self.move_north)
        engine.register_input_callback(engine.INPUT_RIGHT, self.move_east)
        engine.register_input_callback(engine.INPUT_DOWN, self.move_south)
        engine.register_input_callback(engine.INPUT_LEFT, self.move_west)

    """ game engine features (public)
    These are methods which the game engine will execute at the commented moments.
    This will all be autofilled by the creation script with super filled in to help
    prevent errors when it comes to this.
    """

    """ This method is run every frame before the graphics are displayed.
    You can put code here you want to run before every frame.
    You MUST but super().beforeFrameUpdate() for this to work. Otherwise this may lead
    to unexpected behaviour. MAY NOT BE NEEDED, may be able to do everything with callbacks!
    """
    #def before_frame_update(self):
        #super().before_frame_update()

    """ public:
    Put the regular public methods you wish to use here.
    """

    def run_script(self):
        """ Runs the current script in the player_scripts folder in a seperate thread. Exposes the PyGuide API to the script to allow it to control this player. :)

        Everything in the API is bocking, however this doesn't impact regular gameplay as it's run in a seperate thread.
        The callback is run after the script has finished running.

        TODO: work out if the callback should know if the script failed or not.
        (Or if callback is even needed)

        Parameters
        ----------
        script_name : str
            The name of the script you wish to running, in the player_scripts folder in the root of the game.
        callback : function
            The callback function you wish to run after the script has finished runnning.
        """
        if not(self.__running_script): #only run script if one currently isn't running.
            self.__running_script = True # running script TODO: make this system a lot more robust
            scriptrunner.start(self, "current")
        return

    def halt_script(self):
        """ Halts the player script that is running.

        Works by sending the thread the script is running in an Exception, which the thread catches and appropriately handles and
        stops running.
        """
        if(self.__running_script):
            thread_id = self.__thread_id #TODO: Make this process safer, look at temp.py and add appropriate guards around the next line to check for valid results etc.
            res = ctypes.pythonapi.PyThreadState_SetAsyncExc(ctypes.c_long(thread_id), ctypes.py_object(scriptrunner.HaltScriptException))

    def set_running_script_status(self, status):
        """ Set the script runnin status of the player, used by scriptrunner.py as a simple check to see if this player is already running as script.
        
        Simply prevents to scripts with player inputs from running simultaneously.
        """
        self.__running_script = status
        return

    def set_thread_id(self, thread_id):
        self.__thread_id = thread_id
        return

    def get_thread_id(self):
        return thread_id

    """ private:
    Put the private methods you wish to use here.
    """
    
    """ This method takes the movement input of the player character and returns the appropriate
    function for moving them in the direction required
    face_x -- self.face_north/east/south/west() as appropriately required to get them to face in that direction
    """
    #def __handle_movement_input(self, is_facing_x, face_x, move_x):
    #	def handle_input:
    #		if(not(self.moving())):  #can't register input if the character is in the middle of moving
    #			if(is_facing_x()): #if facing in x direction, get them to move in that direction, else face in that direction first  
    #				move_x()
    #			else:
    #				face_x()
    #	return handle_input


