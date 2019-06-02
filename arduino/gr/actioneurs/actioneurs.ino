#include "consts.h"
#include "PololuA4983.h"
#include <Servo.h>

#include <ros.h>
#include <game_manager/GameStatus.h> 
#include <ard_gr_front/PucksTake.h>
#include <ard_gr_front/RaiseThing.h>
#include <ard_gr_front/ArduinoToAI.h>


ros::NodeHandle nh;
PololuA4983 stepper_pucks_door = PololuA4983(PUCKS_DOOR_STEP_PIN, PUCKS_DOOR_DIR_PIN,
                                            PUCKS_DOOR_EN_PIN, PUCKS_DOOR_MIN_DELAY);
Servo selector;

int game_status = GAME_OFF;
int pucks_door_steps_taken = 0;

bool busy = false;
bool puck_to_scale[3];
static int PUMP[3] = {PUMP_1,PUMP_2,PUMP_3};


// Publisher 
ard_gr_front::ArduinoToAI  event_msg ; 
ros::Publisher pub_response  ("actionneurs/ard_gr_front/event", &event_msg); 

void publish_response (int event_type, bool success){
    event_msg.type = event_type;
    event_msg.success = success;
    pub_response.publish(&event_msg);
}

void initialize(){
    selector.write(SELECTOR_TO_TOWER);
}

void stop_elec(){
    stepper_pucks_door.disable();
    digitalWrite(TOWER_PIN, LOW);
    digitalWrite(SCALE_DOOR_PIN, LOW);
    for (int i=0; i<3; i++){
        digitalWrite(PUMP[i], LOW);
    }
    busy = false;
}

void on_game_status(const game_manager::GameStatus& msg){
    //Hardcoded to GAME_ON FOR TESTING
    game_status = GAME_ON;//msg.game_status; 
    if (game_status != GAME_ON)
        stop_elec();

    if (msg.init_status == 1 && game_status == GAME_ON)
        initialize(); 
}
  
void suck_up_pucks() {
    for(int i = 0; i < 3; i++){
        digitalWrite(PUMP[i], HIGH);
    }
    delay(PUMP_DELAY);  
}

void on_take_pucks(const ard_gr_front::PucksTake& msg){
    nh.loginfo("Taking pucks...");
    if (game_status != GAME_ON || busy){
        publish_response(EVENT_PUCKS_TAKE, false);
        return;
    }
    busy = true;

    puck_to_scale[0] = msg.p1;
    puck_to_scale[1] = msg.p2;
    puck_to_scale[2] = msg.p3;

    suck_up_pucks();
    busy = false;
    publish_response(EVENT_PUCKS_TAKE, true);
}

void camion_poubelle(){    
    int compensate = 35;
    for(int i = 0; i < 30; i++){
        pucks_door_goes_up(CAMION_POUBELLE_STEP, false);
        delay(10);
        pucks_door_goes_up(CAMION_POUBELLE_STEP, true);
        delay(10);
        compensate -= 6;
        if (digitalRead(PUCKS_DOOR_UP_SWITCH_PIN) == HIGH){
            pucks_door_goes_up(CAMION_POUBELLE_STEP, false);
            return;
        }
    }
}

void free_puck_to_sort(int i) {
    digitalWrite(PUMP[i], LOW);
    delay(PUCK_TIME_TO_MOVE);
    camion_poubelle();
}

void pucks_door_goes_up(int steps, bool go_up) {    
    stepper_pucks_door.moveStep(steps, go_up);
    
    while(stepper_pucks_door.getRemainingStep() != 0 && game_status == GAME_ON) {
        stepper_pucks_door.update();
        nh.spinOnce();
    }
    
    //nh.loginfo("Pucks door has moved");
}

void dump_in_scale(){
    for(int i = 1; i < 3; i++){
        if(puck_to_scale[i] && game_status == GAME_ON)
           free_puck_to_sort(i);
    }
    if(puck_to_scale[0] && game_status == GAME_ON)
       free_puck_to_sort(0);
}

void dump_in_tower(){
    for(int i = 1; i < 3; i++){
        if(!puck_to_scale[i] && game_status == GAME_ON)
            free_puck_to_sort(i);    
    }
    if(!puck_to_scale[0] && game_status == GAME_ON)
       free_puck_to_sort(0);
}

void pucks_door_all_the_way_up(){
    if (digitalRead(PUCKS_DOOR_UP_SWITCH_PIN) == HIGH)
        //security
        return;

    stepper_pucks_door.moveStep(5000, true);
    while (stepper_pucks_door.getRemainingStep() != 0 &&
        game_status == GAME_ON &&
        digitalRead(PUCKS_DOOR_UP_SWITCH_PIN) != HIGH){
            stepper_pucks_door.update();
            nh.spinOnce();
    }
    stepper_pucks_door.stop();
}

void raise_and_sort_pucks() {
    nh.loginfo("Raising and sorting...");
    stepper_pucks_door.enable();

    if (puck_to_scale[1])
      selector.write(SELECTOR_TO_SCALE);
    else
      selector.write(SELECTOR_TO_TOWER);
      
    pucks_door_all_the_way_up();
    pucks_door_goes_up(50, false);

    if (puck_to_scale[1]){
      dump_in_scale();
      selector.write(SELECTOR_TO_TOWER);
      delay(SELECTOR_TIME_TO_MOVE);
      dump_in_tower();
    }
    else{
      dump_in_tower();
      selector.write(SELECTOR_TO_SCALE);
      delay(SELECTOR_TIME_TO_MOVE);
      dump_in_scale();
    }

    nh.loginfo("all the waying");
    pucks_door_all_the_way_up();
    nh.loginfo("all the way'd");
    pucks_door_goes_up(PUCKS_DOOR_STEP_NB, false);
    stepper_pucks_door.disable();
    
    publish_response(EVENT_PUCKS_RAISE_SORT, true);
}


void raise_scale_door(){
    nh.loginfo("Raising scale door...");
    digitalWrite(SCALE_DOOR_PIN, HIGH);
    int time_counter = 0;
    while (digitalRead(SCALE_DOOR_LIMIT_PIN)!= HIGH && 
        time_counter <= 40 && 
        game_status == GAME_ON) {
            
        delay(50);
        time_counter++;
        nh.spinOnce();
    }
    digitalWrite(SCALE_DOOR_PIN, LOW);
    
    publish_response(EVENT_RAISE_SCALE_DOOR, true);
}   

void raise_tower(){
    nh.loginfo("Raising tower....");

    digitalWrite(TOWER_PIN, HIGH);
    while (digitalRead(TOWER_LIMIT_PIN)!= HIGH && game_status == GAME_ON) {   
        nh.spinOnce();
    }
    digitalWrite(TOWER_PIN, LOW);
    
    publish_response(EVENT_RAISE_TOWER, true);
}

void on_raise_thing(const ard_gr_front::RaiseThing& msg){
    if (game_status != GAME_ON || busy){
        switch(msg.thing_to_raise){
            case RAISE_AND_SORT_ORDER:
                publish_response(EVENT_PUCKS_RAISE_SORT, false);
                break;
            case RAISE_TOWER_ORDER:
                publish_response(EVENT_RAISE_TOWER, false);
                break;
            case RAISE_SCALE_DOOR_ORDER:
                publish_response(EVENT_RAISE_SCALE_DOOR, false);
                break;
        }
        return;
    }

    busy = true;
    switch(msg.thing_to_raise){
        case RAISE_AND_SORT_ORDER:
            raise_and_sort_pucks();
            break;
        
        case RAISE_TOWER_ORDER:
            raise_tower();
            break;
        
        case RAISE_SCALE_DOOR_ORDER:
            raise_scale_door();
            break;
        
        default:
            nh.logwarn("WARNING : incorrect raise msg : must be 0,1,2");
    }
    busy = false;
}
//Subscribers
ros::Subscriber<game_manager::GameStatus>      sub_game_status("/ai/game_manager/status",&on_game_status);

ros::Subscriber<ard_gr_front::RaiseThing>      sub_raise_thing("/actionneurs/ard_gr_front/raise_thing", &on_raise_thing);
ros::Subscriber<ard_gr_front::PucksTake>       sub_take_pucks("/actionneurs/ard_gr_front/take_pucks",&on_take_pucks);

void connectPins(){

    pinMode(PUMP_1, OUTPUT);
    pinMode(PUMP_2, OUTPUT);
    pinMode(PUMP_3, OUTPUT);

    pinMode(PUCKS_DOOR_DIR_PIN, OUTPUT);
    pinMode(PUCKS_DOOR_STEP_PIN, OUTPUT);
    pinMode(PUCKS_DOOR_EN_PIN, OUTPUT);

    pinMode(SCALE_DOOR_PIN, OUTPUT);
    pinMode(TOWER_PIN, OUTPUT);

    pinMode(SCALE_DOOR_LIMIT_PIN, INPUT);
    pinMode(TOWER_LIMIT_PIN, INPUT);

    selector.attach(SELECTOR_PIN);

}

void connectROS(){
    nh.initNode();

    nh.subscribe(sub_game_status);
    
    nh.subscribe(sub_take_pucks);
    nh.subscribe(sub_raise_thing);

    nh.advertise(pub_response);

}
void setup(){
    connectPins();
    connectROS();
}


void loop(){
    nh.spinOnce();
}
