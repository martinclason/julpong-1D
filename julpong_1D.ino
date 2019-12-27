/* 1D Pong - an infinitely narrow pong with an infinite number of bounces.
 * 
 * Rules:
 *  - Hit your button while your light is lit, if you're too early or to late you lose and the other person wins.
 *  - The side that lost will serve the ball next round.
 * 
 * Functions:
 *  - The lamps on the winning side will light up after a round ends.
 *  - If you turn the potentiometer the starting speed of the ball will increase.
 *  - The speed of the ball will increase for every SPEED_INCREASE_INTERVAL´th bounce. 
 *  - If you get over HIGHSCORE_REGISTER_LIMIT number of collective bounces, these are displayed in binary
 *    format.
 *  - If you get a score higher than HIGHSCORE_REGISTER_LIMIT and the current highest score, the display flashes 
 *    before the result is showed.
 *    
 * Have fun!
 */


#define ORANGE_BTN 10
#define GREEN_BTN 11
#define ORANGE_END 2
#define GREEN_END 9
#define PIN_POS_OFFSET 2
#define POT_PIN A0
#define SPEED_INCREASE 8
#define HIGHSCORE_REGISTER_LIMIT 4
#define FLASH_PERIOD 200
#define SPEED_INCREASE_INTERVAL 3

bool isGoingTowardsOrange {false};
int position {0};
bool isOrangeWinner {false};
int result {0};
int highestScore{0};

void moveBallToOrange()
{
    if (position == 0)
    {
        position = 1;
        isGoingTowardsOrange = false;
    }
    else
    {
        position -= 1;
    }
}

void moveBallToGreen()
{
    if (position == 7)
    {
        position = 6;
        isGoingTowardsOrange = true;
    }
    else
    {
        position += 1;
    }
}

bool orangeIsPressed()
{
    return digitalRead(ORANGE_BTN);
}

bool greenIsPressed()
{
    return digitalRead(GREEN_BTN);
}

bool mayBounceOrange()
{
    return position == 0 && isGoingTowardsOrange;
}

bool mayBounceGreen()
{
    return position == 7 && !isGoingTowardsOrange;
}

void updateLamps(int pos)
{
    for (int l {ORANGE_END}; l <= GREEN_END; l++)
    {
        if (l == pos + PIN_POS_OFFSET)
        {
            digitalWrite(l, HIGH);
        }
        else
        { 
            digitalWrite(l, LOW);
        }
    }
}

void orangeLost()
{
  isOrangeWinner = false;
   Serial.println("Orange lost!");
}

void greenLost()
{
  isOrangeWinner = true;
  Serial.println("Green lost!");
}

void displayWinner(bool isOrangeWinner)
{
  if (isOrangeWinner)
  {
    for (int pin {ORANGE_END}; pin <= GREEN_END; pin++)
    {
      if (pin - PIN_POS_OFFSET < 4 || pin == GREEN_END)
      {
        digitalWrite(pin, HIGH);
      }
      else
      {
        digitalWrite(pin, LOW);
      }
    }
  }
  else
  {
    for (int pin {ORANGE_END}; pin <= GREEN_END; pin++)
    {
      if (pin - PIN_POS_OFFSET >= 4 || pin == ORANGE_END)
      {
        digitalWrite(pin, HIGH);
      }
      else
      {
        digitalWrite(pin, LOW);
      }
    }
  }
}

// Displays result in binary format with least significant bit at position ORANGE_END;
void displayResultInBinary(int result)
{
  int remainingResult {result};
  for (int pin{GREEN_END}; pin >= ORANGE_END; pin--)
  {
    int p {1 << (pin - PIN_POS_OFFSET)};
    if (p <= remainingResult)
    {
      digitalWrite(pin, HIGH);
      remainingResult -= p;
    }
    else
    {
      digitalWrite(pin, LOW);
    }
  }
}

void displayFlashes(int nFlashes)
{
  for (int n{0}; n < nFlashes; n++)
  {
    for (int pin{ORANGE_END}; pin <= GREEN_END; pin++)
    {
      digitalWrite(pin, HIGH);
    }
    delay(FLASH_PERIOD / 2);
    for (int pin{ORANGE_END}; pin <= GREEN_END; pin++)
    {
      digitalWrite(pin, LOW);
    }
    delay(FLASH_PERIOD);
  }
}

int scaleToDelay(int value)
{
  return value / 8 + 50;
}

int increaseSpeed(int speed_up)
{
  return speed_up + SPEED_INCREASE;
}

// runs one round and returns number of successful bounces
int playRound()
{
  int speedUp {0};
  int currentDelay {120};
  int successfulBounces {0};
  
  if (isOrangeWinner)
  {
    isGoingTowardsOrange = true;
    position = 7; 
  }
  else 
  {
    isGoingTowardsOrange = false;
    position = 0; 
  }
  
  bool lost {false};
  while (!lost)
  {
  // check orange side
  if (mayBounceOrange())
    { // check that orange is not pressing the button too late
        if (!orangeIsPressed())
        {
            orangeLost();
            lost = true;
            break;
        }
        else
        {          
          successfulBounces++;
          if (successfulBounces % SPEED_INCREASE_INTERVAL == 0)
          {
            speedUp = increaseSpeed(speedUp);
          }
        }
    }
    else
    { // check that orange is not pressing the button too early
      if (orangeIsPressed() && isGoingTowardsOrange)
      {
          orangeLost();
          lost = true;
          break;
      }
    }

    // check green side
    if (mayBounceGreen())
    { // check that green is not pressing the button too late
        if (!greenIsPressed())
        {
            greenLost();
            lost = true;
            break;
        }
        else
        {
          successfulBounces++;
          if (successfulBounces % SPEED_INCREASE_INTERVAL == 0)
          {
            speedUp = increaseSpeed(speedUp);
          }
        }
    }
    else
    { // check that orange is not pressing the button too early
      if (greenIsPressed() && !isGoingTowardsOrange)
      {
          greenLost();
          lost = true;
          break;
      }
    }

    // update ball position
    if (isGoingTowardsOrange)
    {
        moveBallToOrange();
    }
    else
    {
        moveBallToGreen();
    }

    updateLamps(position);
    
    // read current speed from potentiometer
    currentDelay = scaleToDelay(analogRead(POT_PIN));
    delay(currentDelay - speedUp);
    Serial.println(currentDelay);
  }
  return successfulBounces;
}

void setup() {
    // Setup pin modes
    pinMode(ORANGE_BTN, INPUT);
    pinMode(GREEN_BTN, INPUT);
    for (int pin {ORANGE_END}; pin <= GREEN_END; pin++)
    {
        pinMode(pin, OUTPUT);
    }

    Serial.begin(9600);
}

void loop() {
    // run round
    result = playRound();

    displayWinner(isOrangeWinner);
    delay(2000);
    
    if (result >= HIGHSCORE_REGISTER_LIMIT)
    {
      if (result > highestScore)
      {
        displayFlashes(4);
        highestScore = result;
      }
      displayResultInBinary(result);
      delay(2000);
    }
}
