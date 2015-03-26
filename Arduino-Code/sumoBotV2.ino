//declaring global variables
const unsigned int RSonarpinOut = 13; // sonar pins
const unsigned int RSonarpinIn = 12;
const unsigned int LSonarpinOut = 8;
const unsigned int LSonarpinIn = 2;

const unsigned int mlforward = 3;    // motor pins
const unsigned int mlback = 4;
const unsigned int mrforward = 7;
const unsigned int mrback = 6;

const unsigned int LData = 0; // for collecting data of if thing was detected in front of sensor. Value either 1 or 0
const unsigned int RData = 0;

long timel; //time received from left sonar
double distl; //distance from left sonar to object
long timer; //same as left
double distr; //same as left

double Ldist; //used to store value from seek functions
double Rdist; 

double LRdif; //used to check if the object is closer to the left or right

int qrdL; //true of false to check if a white line is detected
int qrdR; 

int QL; //used to store values returned from QRD functions
int QR;

void setup() {
  pinMode(12,INPUT);              // sonar right input
  pinMode(13, OUTPUT);            // sonar right output
  pinMode(8, OUTPUT);             // sonar left output
  pinMode(2, INPUT);              // sonar left input
  pinMode(mlforward, OUTPUT);     //motors
  pinMode(mlback, OUTPUT);
  pinMode(mrforward, OUTPUT);
  pinMode(mrback, OUTPUT);
  pinMode(A0 , INPUT);            //Left QRD
  pinMode(A1, INPUT);             //Right QRD
  Serial.begin(9600);             //Opens serial monitor to check values(not needed in comp but useful for testing)
  delay(6000);// 5 second rule of competition
  forward(); //moves robot forward to try and get closer at the beginning of the competition
  delay(1500); //keeps moving forward for 1sec before the seeking looping 
}

void loop() {//looping through the robot logic
  QR = qrdRight(); //checks if the QRD sense white (meaning at the edge of the ring and need to move backwards
  QL = qrdLeft(); //most important part of the loop
  
  if (QR==1 || QL==1) {//if one of the QRDs sense white move backward
    backward();
    delay(2000);
    brake();
    
    if (QL==1 && QR==0) {//if only the left sensor is "tripped" then need to turn to the right
      HardRight();       //as it insures the robot moves as far away from border as possible
      delay(2000);
      brake();
    }
    
    else {        //if only right sensor is tripped then need to turn left. turning left is also the defualt sequence if both are tripped
      HardLeft(); //which is why it is handled all in one else statement
      delay(2000);
      brake();
    }
  }
  
  else{ //if none of the QRDs sense white then the robot is free to move and seek out the opponent
    
  Rdist = seekR(); //gets Right distance
  Ldist = seekL(); //gets Left distance
  
  if (Ldist<=40 && Rdist>40) {	//opponent is very far on the left, therefore needs to move to the left
  HardLeft(); //hard left is called since opponent is very far to the left
  delay(500); //short turn to get a better reading on the next loop
  forward();  //continue to move forward until next loop
  }
  
  else if (Rdist<=40 && Ldist>40) {//same as above but on the right side
  HardRight();
  delay(500);
  forward();
  }
  
  else if (Rdist<=40 && Ldist<=40) {//both sensors are detecting object
    //check too see which one is closer
    LRdif = Ldist - Rdist; //finds the difference in distance
    if (LRdif <= 10 && LRdif >= -10) {//object is right in front, call forward
      forward();
      
      
    if (Rdist<=15 && Ldist<=15) { //this checks to see if the object is VERY close. if so we want our robot try and push
                                //opponent out of the ring while we still have them in our sights
        while (1){      //this allows the robot to move forward until a QRDis tripped ensuring that the robot either pushes the opponent out             
          QL=qrdLeft(); //or stops when it gets to the edge of the ring
          QR=qrdRight();
          if (QL==1 || QR==1) {//senses that the QRDs have been tripped. same as the intial QRD check at the beggining of the loop
            backward();        //however also breaks the while loop so that the robot can go back to scanning for the opponent
            delay(2000);
           if (QL==1 && QR==0) {//chooses which way the robot needs to turn. same as initial QRD check
              HardRight();
              delay(2000);
               brake();
               break;
               }
      
           else {
             HardLeft();
             delay(2000);
             brake();
             break;
           } 
          }
        }
    }
      
       
    } //end of checking if the robot is directly in front, move on to checking fringe cases (in-between forward and hardturns)
    
    else if (LRdif >7) {//left dist is larger than right dist (closer to right)
                        //call softRight since it is closer to  the right but not a large amount
      SoftRight();
    }
    
    else {//right dist is larger than left dist (closer to left)
          //same reasoning as above
      SoftLeft();
    }
  }
  
  else {//nothing found, turn arbitrary directions
    Look();
  }
  
}
}//end of loop function. once it reaches this point it will return back to the beginning and repeat until the SumoBot ma





int seekR(){ //seeks right, returns a distance value
  //emit s sound wave
  
  digitalWrite(RSonarpinOut,LOW); //this block is for emitting, and receiving the sonar pulse
  delayMicroseconds(2);
  digitalWrite(RSonarpinOut,HIGH);
  delayMicroseconds(5);
  digitalWrite(RSonarpinIn,HIGH);
  
  //receive a sound wave
  timer = pulseIn(RSonarpinIn,HIGH);
  
  //convert time (microsec) to distance in cm
  //given the speed of sound is 340 m/s
  distr = timer / 29 / 2;
  Serial.print("RDist: \t"); //outputs the value to the serial monitor. not used for the match
  Serial.print(distr);       //only used for testing
  return distr; //returns the distance to be compared in the robot logic
}

int seekL() {//same as right but for left side
  //emit a sound wave
  
  digitalWrite(LSonarpinOut,LOW);
  delayMicroseconds(2);
  digitalWrite(LSonarpinOut,HIGH);
  delayMicroseconds(5);
  digitalWrite(LSonarpinOut,LOW);
  
  timel = pulseIn(LSonarpinIn,HIGH);
  
  distl = timel / 29 / 2;
  Serial.print("LDist: \t");
  Serial.println(distl);
  return distl;
}

void forward(){ // set both motors full speed forward
  analogWrite(mlback, 0);
  analogWrite(mrback,0);
  analogWrite(mlforward, 255);
  analogWrite(mrforward, 255);
}

void backward(){ //set both motors to full speed reverse
  analogWrite(mlforward, 0);
  analogWrite(mrforward,0);
  analogWrite(mlback, 255);
  analogWrite(mrback, 255);
}

void brake(){ //stops both motors
  analogWrite(mlforward, 0);
  analogWrite(mlback, 0);
  analogWrite(mrforward, 0);
  analogWrite(mrback, 0);
}

void HardLeft(){ //used for making a very large turn to the left
  analogWrite(mlforward, 0);
  analogWrite(mrback,0);
  analogWrite(mlback, 100);
  analogWrite(mrforward, 255);
}

void HardRight(){ //same as HardLeft but to the right
  analogWrite(mlback, 0);
  analogWrite(mrforward,0);
  analogWrite(mrback, 100);
  analogWrite(mlforward, 255);
}

void SoftLeft(){ //used to make a smaller turn to the left
  analogWrite(mlback, 0);
  analogWrite(mrback,0);
  analogWrite(mlforward,255);
  analogWrite(mrforward,150);
}

void SoftRight(){ //same as SoftLeft but to the right
  analogWrite(mlback, 0);
  analogWrite(mrback,0);
  analogWrite(mrforward, 255);
  analogWrite(mlforward, 150);
}

void Look(){ //signals the robot to start scanning (turning to the left by default)
  analogWrite(mlback, 0);
  analogWrite(mrback,100);
  analogWrite(mrforward, 0);
  analogWrite(mlforward, 255);
}

int qrdLeft(){ //check if at white line
  qrdL = analogRead(A0); 
  if (qrdL<=650) {return 1;} //low value = white detected-->return 1
  else {return 0;}           //high value = black detected-->return 0
}

int qrdRight(){ //same as qrdLeft but for the other sensor
  qrdR = analogRead(A1);
  if (qrdR<=650) {return 1;} 
  else {return 0;}
}


      

