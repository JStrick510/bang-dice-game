#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <time.h>
#include <iostream>
#include <math.h>
#include <fstream>
//#include <malloc/_malloc.h> // instead of "#include <malloc.h>" use this for mac
#include <malloc.h>
#include <conio.h>
using namespace std;

// structure for the player
//structure for the player
struct player 
{
    int currentHealthPoints; //curent health points of the player
    int maxHealthPoints; //maximum health points of the player
    int role; //the role assigned to the player
    int arrowCount = 0; //current number of arrows the player has
    int playerIndex; //where the player is in the list aka table position
    int rerollsRemaining = 2; //stores amount of rerolls player has left in turn, should be reset to two at end of turn
    struct favorList *next = NULL;
};

//structure for player favor
struct favorList
{
    int favorIndex;
    int favorValue;
    struct favorList *next;
};

//structure for the dice
struct dice
{
    int reroll; //value of dice for rerolling
    int value;  //value of dice 
    int result[5]; //current dice statuses
};
struct dice start_to_roll;  //declare the struct name

//structure for the circular doubly linked list of the player struct
struct node
{
    struct player data; //player that is stored in the list
    struct node *next, *prev; //pointer to the next item and previous item in the list respectively
};
struct node *start = NULL; //pointer to the start of the player list

//delcaration of functions used for player list
struct node *insertList(struct player); //function is able to create list and then inserts elements to the end of the list
struct node *deleteNode(struct player); //deletes a node based on a given player, assumption that node asked to be deleted will always be in list
struct node *displayList(); //function needed for testing, assumping that list is never empty, maybe could be modified to display game status

//declaration of functions used for favor list
struct favorList *insertFavor(struct player basePlayer, struct player insertPlayer);
struct favorList *deleteFavor(struct player);
void changeFavor(struct player basePlayer, int changeIndex, int favorDiff);
void shotFavor(struct player shotPlayer, struct player shootingPlayer);
void healFavor(struct player healedPlayer, struct player healingPlayer);
void sortFavor(struct player basePlayer);
//declaration of functions used for dice
int roll_dice(struct dice); // to roll dice five times 
int reroll_dice(struct dice, int); // to reroll a specific dice
void display_dice(struct dice); // display current dice status 

//declaration of functions for dice functionality
void beerDice(struct node *playerTurn);
void arrowDice(struct node *playerTurn, int DiceVal);
void indianAttack();
void gatlingDice(struct node *playerTurn);
void dynamiteDice(struct node *playerTurn);
void check_ShotDice(struct node *playerTurn, int result[]);
void shot_next(struct player playerTurn, int direction);
void shot_second(struct player playerTurn, int direction); 

//declaration of functions used for player setup
struct player playerInfoSetup(int i); //This function creates and initializes the players and their information 
void gameSetup(int totalPlayers); //This function is to initialize values that need to be done before the game begins

//declaration of functions used for checking
int isPlayerDead(struct player); //function that checks if player is dead, 0 if not dead and 1 if dead
void isGameOver(); //function checks if game is over, changes global game over flag

//declaration of auxillary functions
void printRole(int outputRole); //takes the numerical role value and prints out what that role is
int checkFreq(int result[], int diceNum); //returns the number of occurances of a given dice
int findFavor(struct player basePlayer, int searchPlayerIndex); //finds the favor of a given player

//gobal variables
int sher = 1;
int depu, outl, reneg;
int roles[] = {sher, reneg, outl, depu}; //array to store the number of each role left in the game
int gameOver = 0; //0 if game is continuing, 1 if game is over
int arrowPile = 9; //total arrows in the center
int diceReroll[5] = {0, 0, 0, 0, 0};
int totalPlayers;
int playerDied;

int main()
{
    cout << "Bang! Dice Game" << endl << "Jacob Strickland, Airi Shimamura, Rodrigo Gutierrez" << endl << endl;
    srand(time(NULL));
    //Generates the total number of players at the start of the game. Too simple to merit its own function
    totalPlayers = rand() % 5 + 4;

    cout << "There are a total number of players: "<< totalPlayers << endl;
    
    gameSetup(totalPlayers);//If there are any other things that need to be initilalized before the game begin do that in this function
    struct player temp;//I create a struct to reference the current player that is to be inserted into the list. 
    for(int i = 0; i < totalPlayers; i++)
    {
        temp = playerInfoSetup(i);//playerInfoSetup returns the struct of the new player and is pointed to by ptr 
        start = insertList(temp); 
    }
    
    struct node *ptrX, *ptrY;
    ptrX = start;
    for(int x = 0; x < totalPlayers; x++)
    {
        ptrY = start;
        for(int y = 0; y < totalPlayers; y++)
        {
            if(ptrY -> data.playerIndex != ptrX -> data.playerIndex)
                ptrX -> data.next = insertFavor(ptrX -> data, ptrY -> data);
            ptrY = ptrY -> next;
        }
        ptrX = ptrX -> next;
    }
    
	int z;
    
    start = displayList(); //at the start of the game
     
    //basic turn algorithm
    struct node *ptr, *findNext, *checkDeath, *findPlayer;
    ptr = start;
    
    struct favorList *ptr1;
    
    while(ptr -> data.role != 0) //to find the sheriff since they go first
        ptr = ptr -> next;
    
    while(!gameOver)
    {
        cout << "Player " << ptr -> data.playerIndex << " Turn Start:" << endl << endl;
        int n = (ptr -> data.playerIndex + 1) % (totalPlayers+1);
        playerDied = 0;
        
        cout << "Dice Status:" << endl;
        roll_dice(start_to_roll); //roll the dice the first time
        
        if(checkFreq(start_to_roll.result, 2) >= 3) //if 3 dynamite the player cant reroll anymore
            ptr -> data.rerollsRemaining = 0;
        cout << endl;
        for(int arr = 0; arr < 5; arr++ )
        {
            if(playerDied == 0)
            {
                arrowDice(ptr, start_to_roll.result[arr]);
                if(isPlayerDead(ptr -> data))
                    playerDied = 1;
            }
        }
        
        while(playerDied == 0 && ptr -> data.rerollsRemaining > 0 && gameOver != 1) //if player can reroll the dice again, start the reroll process
        {
            int j;
            cout << endl << "Rerolling Dice: " << endl;
            for(j = 1; j <= 5; j++) //randomly choose dice to reroll, 50/50 chance
            {
                int chooseReroll = 0;
                if(start_to_roll.result[j-1] == 6 && playerDied == 0 && gameOver != 1) //if beer
                {
                    if(checkFreq(start_to_roll.result, 6) + ptr -> data.currentHealthPoints <= ptr -> data.maxHealthPoints) //if player still can be healed
                        chooseReroll = 1; //dont reroll
                    else
                    {
                        ptr1 = ptr -> data.next;
                        while(ptr1 != NULL)
                        {
                            if(ptr1 -> favorValue >= 50) //if there is a player that has 50 favor with the player
                            {
                                findPlayer = start;
                                for(int a = 0; a < totalPlayers; a++) //go through the list of players
                                {
                                    //if that player is below half health
                                    if(findPlayer -> data.playerIndex == ptr1 -> favorIndex && findPlayer -> data.currentHealthPoints < findPlayer -> data.maxHealthPoints / 2)
                                        chooseReroll = 1; //dont reroll
                                    findPlayer = findPlayer -> next;
                                }
                            }
                            ptr1 = ptr1 -> next;
                        }
                    }
                }
                else if(start_to_roll.result[j-1] == 3 && playerDied == 0 && gameOver != 1)
                {
                    //if player to left or right is in bad favor dont reroll
                    if(findFavor(ptr -> data, ptr -> next -> data.playerIndex) <= -50 || findFavor(ptr -> data, ptr -> prev -> data.playerIndex) <= -50)
                        chooseReroll = 1;
                }
                else if(start_to_roll.result[j-1] == 4 && playerDied == 0 && gameOver != 1)
                {
                    //if player to left or right is in bad favor dont reroll
                    if(findFavor(ptr -> data, ptr -> next -> data.playerIndex) <= -50 || findFavor(ptr -> data, ptr -> prev -> data.playerIndex) <= -50)
                        chooseReroll = 1;
                    //if player two over left or right is in bad favor dont reroll
                    if(findFavor(ptr -> data, ptr -> next -> next -> data.playerIndex) <= -50 || findFavor(ptr -> data, ptr -> prev -> prev -> data.playerIndex) <= -50)
                        chooseReroll = 1;
                }
                else if(start_to_roll.result[j-1] == 5 && playerDied == 0 && gameOver != 1)
                {
                    int gatlingFreq = checkFreq(start_to_roll.result, 5);
                    int randomNum;
                    
                    if(gatlingFreq == 3) //not greater than because want to reroll if more than three
                        chooseReroll = 1;
                    else if(gatlingFreq == 2) //75% chance to keep gatling if have two
                    {
                        randomNum = rand() % 4;
                        if(randomNum > 0)
                            chooseReroll = 1;
                    }
                    else if(gatlingFreq == 1) //33% chance to keep gatling if have one
                    {
                        randomNum = rand() % 3;
                        if(randomNum > 1)
                            chooseReroll = 1;
                    }
                }
                else if(start_to_roll.result[j-1] == 2 && playerDied == 0 && gameOver != 1) //if dynamite might as well keep since cant be rerolled
                    chooseReroll = 1;
                
                if(chooseReroll == 0) //reroll flag not set reroll
                {
                    cout << "Dice " << j << " was rerolled to: ";
                    reroll_dice(start_to_roll, j);
                    arrowDice(ptr,start_to_roll.result[j-1]);
                    if(isPlayerDead(ptr -> data))
                    {
                        playerDied = 1;
                        j = 6;
                    }
                }
                else
                     cout << "Dice " << j << " was chosen to be kept" << endl;
                    
            }
            
            if (!gameOver)
            {
                cout << endl << "New Dice Status:" << endl;
                display_dice(start_to_roll);
            }
            
            if(checkFreq(start_to_roll.result, 2) >= 3) //if player has 3 dynamite the player cant reroll anymore, break out of loop of rerolls
            {
                ptr -> data.rerollsRemaining = 0;
                break;
            }
            else 
                ptr -> data.rerollsRemaining--;
        }
        
        //start resolving the dice       
        if(playerDied != 1 && !gameOver)
        {
            for(int b = 0; b < 5; b++)
            {
                if(start_to_roll.result[b] == 6)
                    beerDice(ptr);
            }
        }
        
        if(playerDied != 1 && !gameOver)
            check_ShotDice(ptr, start_to_roll.result);
        
        if(checkFreq(start_to_roll.result, 2) >= 3 && playerDied != 1 && !gameOver) //resolve dynamite
        {
            dynamiteDice(ptr);
            if(isPlayerDead(ptr -> data))
                playerDied = 1;
        }
        if(checkFreq(start_to_roll.result, 5) >= 3 && playerDied != 1 && !gameOver) //resolve gatling
            gatlingDice(ptr);
        
        if(playerDied == 0 && !gameOver)
            ptr -> data.rerollsRemaining = 2; //reset players reroll value for the next turn
        
        checkDeath = start; //check if the players are dead
        for(int c = 0; c < totalPlayers; c++)
        {
            if(isPlayerDead(checkDeath -> data)) //if player is dead reset to start of list since memory is lost
                checkDeath = start;
            else
                checkDeath = checkDeath -> next;
        }

        if(!gameOver)
        {
            int found = 0;
            for(;;)
            {
                findNext = start;
                do
                {
                    if(findNext -> data.playerIndex == n)
                    {
                        found = 1;
                        break;
                    }
                    findNext = findNext -> next;
                }while(findNext -> next != start);
                if(findNext -> data.playerIndex == n)
                    found = 1;
                
                if(found == 1)
                    break;
                else
                    n = (n + 1) % (totalPlayers+1);
            }
            
            ptr = findNext;
        }
	
        start = displayList(); //display list at end of turn
    }
    
    //getch();
    return 0;
}

struct node *insertList(struct player playerAdd)
{
    struct node *new_node, *ptr; //new_node is the node that will be added, ptr is the node that will traverse the list
    
    new_node = (struct node*)malloc(sizeof(struct node));
    new_node -> data = playerAdd;
    
    if(start == NULL) //if no list created yet
    {
        new_node -> next = new_node; //node has to point to itself since its a circular list
        new_node -> prev = new_node; //node has to point to itself since its a circular list
        start = new_node;
    }
    
    else
    {
        ptr = start;
        while(ptr -> next != start) //end of the list will have the next pointer pointing back to the start
            ptr = ptr -> next;
        new_node -> prev = ptr; //set last node to point back to prev last node
        new_node -> next = start; //set last node to point to start of list
        ptr -> next = new_node; //set prev node to point to new last node
        start -> prev = new_node; //set start to circle to end
    }
    return start;
}

struct node *deleteNode(struct player playerDelete)
{
    struct node *ptr; //ptr will traverse list and act as temp placeholder
    ptr = start;
    
    if(ptr -> data.playerIndex == playerDelete.playerIndex) //if deleting the first node in list, can use player index since its unique to each player
    {
        start -> next -> prev = start -> prev; //the second item in list will point to end
        start -> prev -> next = start -> next; //the last item in list will point to second
        start = start -> next;
        free(ptr);
        totalPlayers--;
    }
    
    else
    {
        while(ptr -> data.playerIndex != playerDelete.playerIndex)
            ptr = ptr -> next;
        ptr -> prev -> next = ptr -> next; //set prev node to point to new next node after deletion
        ptr -> next -> prev = ptr -> prev; //set next node to point to new prev after deletion
        free(ptr);
        totalPlayers--;
    }
    return start;
}

struct favorList *insertFavor(struct player basePlayer, struct player insertPlayer)
{
    struct favorList *new_node, *ptr; //new_node is the node that will be added, ptr is the node that will traverse the list
    new_node = (struct favorList*)malloc(sizeof(struct favorList));
    
    if(basePlayer.role == 2 && insertPlayer.role == 0) //if base is outlaw and insert is sheriff
        new_node -> favorValue = -10000;
    else if(basePlayer.role == 3 && insertPlayer.role == 0) //if base is deputy and insert is sheriff
        new_node -> favorValue = 10000;
    else if(basePlayer.role == 1 && insertPlayer.role != 0) //if base is renegade and insert is not sheriff
        new_node -> favorValue = -25;
    else
        new_node -> favorValue = 0;
    
    new_node -> favorIndex = insertPlayer.playerIndex;
    new_node -> next = NULL;
    
    if(basePlayer.next == NULL) //if no list created yet
        basePlayer.next = new_node;
    
    else
    {
        ptr = basePlayer.next;
        while(ptr -> next != NULL) //end of the list
            ptr = ptr -> next;
        ptr -> next = new_node;
    }
    
    return basePlayer.next;
}

struct favorList *deleteFavor(struct player playerDelete)
{
    //algorithm for deleting dead player's favor list
    struct favorList *ptr, *preptr;
    ptr = playerDelete.next;
    
    while(ptr != NULL)
    {
        if(ptr -> next == NULL)
        {
            free(ptr);
            ptr -> next = NULL;
            ptr = playerDelete.next;
        }
        else
        {
            preptr = ptr;
            ptr = ptr -> next;
        }
    }
    
    //algorithm for deleteing dead player from other favor list
    struct node *listPtr;
    listPtr = start;
    
    for(int i = 0; i < totalPlayers; i++)
    {
        if(listPtr -> data.playerIndex != playerDelete.playerIndex)
        {
            ptr = listPtr -> data.next;
            if(ptr -> favorIndex == playerDelete.playerIndex)
            {
                listPtr -> data.next = ptr -> next;
                free(ptr);
            }
        }
        else
        {
            while(ptr != NULL)
            {
                if(ptr -> favorIndex == playerDelete.playerIndex)
                {
                    preptr -> next = ptr -> next;
                    free(ptr);
                    break;
                }
                preptr = ptr;
                ptr = ptr -> next;
            }
        }
        
        listPtr = listPtr -> next;
    }
}

void changeFavor(struct player basePlayer, int changeIndex, int favorDiff)
{
	if(basePlayer.playerIndex != changeIndex)
	{
		struct favorList *ptr;
		ptr = basePlayer.next;
		
		while(ptr -> favorIndex != changeIndex)
			ptr = ptr -> next;
		
		ptr -> favorValue = ptr -> favorValue + favorDiff;
	}
}

void shotFavor(struct player shotPlayer, struct player shootingPlayer)
{
    struct node *ptr;
    ptr = start;
    int i;
    if(shotPlayer.role == 0) //if sheriff is shot
    {
        changeFavor(shotPlayer, shootingPlayer.playerIndex, -75); //minus favor of shooter by 75 for sheriff
        
        for(i = 0; i < totalPlayers; i++) //minus favor of shooter by 75 for all deputies
        {
            if(ptr -> data.role == 3)
                changeFavor(ptr -> data, shootingPlayer.playerIndex, -75);
                
            ptr = ptr -> next;
        }
        
        ptr = start;
        for(i = 0; i < totalPlayers; i++) //plus favor of shooter by 50 for all outlaws
        {
            if(ptr -> data.role == 2)
                changeFavor(ptr -> data, shootingPlayer.playerIndex, 50);
                
            ptr = ptr -> next;
        }
        
        ptr = start;
        for(i = 0; i < totalPlayers; i++) //minus favor of shooter by 20 for all renegades
        {
            if(ptr -> data.role == 1)
                changeFavor(ptr -> data, shootingPlayer.playerIndex, -20);
                
            ptr = ptr -> next;
        }
    }
    
    else
    {
        for(i = 0; i < totalPlayers; i++)
        {
            if(ptr -> data.playerIndex == shotPlayer.playerIndex) //lower favor of person who shot you by 50
                changeFavor(ptr -> data, shootingPlayer.playerIndex, -50);
            else
            {
                if(ptr -> data.role == 0 || ptr -> data.role == 3) //if not shot increase favor of shooter by 20 for sheriff and deputies
                    changeFavor(ptr -> data, shootingPlayer.playerIndex, 20);
                else //if not shot increase favor of shooter by 10 for outlaws and renegades
                    changeFavor(ptr -> data, shootingPlayer.playerIndex, 10);
            }
            
            ptr = ptr -> next;
        }
    }
}

void healFavor(struct player healedPlayer, struct player healingPlayer)
{
    struct node *ptr;
    ptr = start;
    int i;
    if(healedPlayer.role == 0) //if sheriff is healed
    {
        changeFavor(healedPlayer, healingPlayer.playerIndex, 75); //plus favor of healer by 75 for sheriff
        
        for(i = 0; i < totalPlayers; i++) //plus favor of healer by 75 for all deputies
        {
            if(ptr -> data.role == 3)
                changeFavor(ptr -> data, healingPlayer.playerIndex, 75);
                
            ptr = ptr -> next;
        }
        
        ptr = start;
        for(i = 0; i < totalPlayers; i++) //minus favor of healer by 75 for all outlaws
        {
            if(ptr -> data.role == 2)
                changeFavor(ptr -> data, healingPlayer.playerIndex, -75);
                
            ptr = ptr -> next;
        }
        
        ptr = start;
        for(i = 0; i < totalPlayers; i++) //minus favor of healer by 5 for all renegades
        {
            if(ptr -> data.role == 1)
                changeFavor(ptr -> data, healingPlayer.playerIndex, -5);
                
            ptr = ptr -> next;
        }
    }
    
    else
    {
        for(i = 0; i < totalPlayers; i++)
        {
            if(ptr -> data.playerIndex == healedPlayer.playerIndex) //raise favor of person who healed you by 50
                    changeFavor(ptr -> data, healingPlayer.playerIndex, 50);
            else
            {
                if(ptr -> data.role == 0 || ptr -> data.role == 3) //if not healed decrease favor of healer by 5 for sheriff and deputies
                    changeFavor(ptr -> data, healingPlayer.playerIndex, -5);
                else ///if not healed decrease favor of healer by 5 for outlaws and renegades
                    changeFavor(ptr -> data, healingPlayer.playerIndex, -5);
            }
            
             ptr = ptr -> next;
        }
    }
}

struct node *displayList()
{
    struct node *ptr; //ptr will traverse list
    ptr = start;
    cout << "-------------------------------------------------------------------------------" << endl;
    for(int i = 0; i < totalPlayers; i++)
    {
        cout << "Player " << ptr -> data.playerIndex << " - Max Health: " << ptr -> data.maxHealthPoints << "\tCurrent Health: " << ptr -> data.currentHealthPoints << "\tArrow Count: " << ptr -> data.arrowCount << "\tRole: ";
        printRole(ptr -> data.role);
        cout << endl;
        ptr = ptr -> next;
    }
    cout << "-------------------------------------------------------------------------------" << endl;
    return start;
}

int roll_dice(struct dice){
    
    for(int i = 0;  i < 5; i++){      // to roll five dices 
        cout << i + 1 << ": ";

        start_to_roll.value = rand() % 6 + 1;     // get random numbers
        
        start_to_roll.result[i] = start_to_roll.value; // store random numbers 
        
        switch(start_to_roll.value){
            case 1:
                    cout << "Arrow" << endl;
                    break;
            case 2: 
                    cout << "Dynamite" << endl;
                    break;
            case 3: 
                    cout << "Shot once the next person, "
                         << "to the right or to the left" << endl;
                    break;
            case 4:
                    cout << "Shot once the second person"
                         << " to the right or to the left" << endl;
                    break;
            case 5: 
                    cout << "Gatling gun" << endl;
                    break;
            case 6: 
                    cout << "Beer" << endl;
                    break;                    
         default: 
                cout << "Wrong";
        }
    }
    return 0;
}


int reroll_dice(struct dice, int reroll_num){ //QUESTION: is this supposed to say struct dice start_to_roll? because as this is right now youre just editing the same start_to_roll global struct i think 
    
    if(start_to_roll.result[reroll_num-1] == 2){ // when it's dynamite 
        cout << "Cannot Reroll" << endl;
    }
    else{
        start_to_roll.reroll = rand() % 6 + 1;
        start_to_roll.result[reroll_num-1] = start_to_roll.reroll; // restore random number 
        switch(start_to_roll.reroll){
            case 1:
                    cout << "Arrow" << endl;
                    break;
            case 2: 
                    cout << "Dynamite" << endl; 
                    break;
            case 3: 
                    cout << "Shot once the next person, "
                         << "to the right or to the left" << endl;
                    break;
            case 4:
                    cout << "Shot once the second person"
                         << " to the right or to the left" << endl;
                    break;
            case 5: 
                    cout << "Gatling gun" << endl;
                    break;
            case 6: 
                    cout << "Beer" << endl;
                    break;                    
         default: 
                cout << "Wrong";
            } 
        }
    return 0;
}

void display_dice(struct dice){

    for(int i = 0; i < 5; i++) {
        cout << i + 1 << ": ";
        
         switch(start_to_roll.result[i]) {
            case 1:
                    cout << "Arrow" << endl;
                    break;
            case 2: 
                    cout << "Dynamite" << endl; 
                    break;
            case 3: 
                    cout << "Shot once the next person, "
                         << "to the right or to the left" << endl;
                    break;
            case 4:
                    cout << "Shot once the second person"
                         << " to the right or to the left" << endl;
                    break;
            case 5: 
                    cout << "Gatling gun" << endl;
                    break;
            case 6: 
                    cout << "Beer" << endl;
                    break;                    
            default: 
                    cout << "Wrong"<< endl;
                    break;
        } 
    }
    return;
}

void beerDice(struct node *playerTurn)
{
    struct favorList *favorPtr;
    struct node *currentCheck;
    int nodeIndex;
    int healFlag = 0;
    sortFavor(playerTurn -> data);
    if(playerTurn -> data.currentHealthPoints != playerTurn -> data.maxHealthPoints)
    {
        playerTurn -> data.currentHealthPoints++;
        cout << "Player "<< playerTurn -> data.playerIndex << " healed themselves. Their current health is now "<< playerTurn -> data.currentHealthPoints<< ". " << endl;
        healFavor(playerTurn -> data, playerTurn -> data);
        healFlag = 1;
    }
    else
    {
        sortFavor(playerTurn -> data);
        favorPtr = playerTurn -> data.next;
        for(int i = 0; i < totalPlayers - 1; i++)
        {
            currentCheck = start;
            nodeIndex = favorPtr -> favorIndex;
            while(currentCheck -> data.playerIndex != nodeIndex)
            {
                currentCheck = currentCheck -> next;
            }
            if(currentCheck -> data.currentHealthPoints != currentCheck -> data.maxHealthPoints)
            {
                currentCheck -> data.currentHealthPoints++;
                cout << "Player "<< playerTurn -> data.playerIndex << " healed player " << currentCheck -> data.playerIndex << ". Their current health is now "<< currentCheck -> data.currentHealthPoints<< ". " << endl;
                healFavor(currentCheck -> data, playerTurn -> data);
                healFlag = 1;
            }
            if(healFlag == 1)
                break;
            else
            {
                favorPtr = favorPtr -> next;
            }
        }
    }
    
    if(healFlag == 0)
        cout << "Nobody was available to heal" <<  endl;
    
        
}
void arrowDice(struct node *playerTurn, int DiceVal)
{
        if(DiceVal == 1)
        {
            playerTurn -> data.arrowCount++;
            cout << "Player " << playerTurn -> data.playerIndex << " takes an arrow, their arrown count is now " << playerTurn -> data.arrowCount << endl;
            arrowPile--;
            cout << "The number of arrows in the pile is now down to " << arrowPile << endl;
           
            if(arrowPile == 0)
            {
                cout << endl;
                indianAttack();
                cout << endl;
            }
        }
}

void indianAttack()
{
    struct node *ptr;
    ptr = start;
    int temp;
    
    cout << "Indian Attack!" << endl;
    int indxCount = 0;
    do
    {
        if(ptr -> data.arrowCount != 0)
            cout << "Player " << ptr -> data.playerIndex << " has " << ptr -> data.arrowCount << ". Current HP " << ptr -> data.currentHealthPoints - ptr -> data.arrowCount << endl;
            
        ptr -> data.currentHealthPoints -= ptr -> data.arrowCount;
        ptr -> data.arrowCount = 0;
        ptr = ptr -> next;
        indxCount++;
    }while (indxCount <= totalPlayers && gameOver != 1);
    
    arrowPile = 9;
}

void gatlingDice(struct node *playerTurn)
{
    //return arrows from player to the pile
    int returnedArrows = 0;
    returnedArrows = playerTurn -> data.arrowCount;
    arrowPile = arrowPile + returnedArrows;
    playerTurn -> data.arrowCount = 0;
    
    cout << "Player " << playerTurn -> data.playerIndex << " has 3+ gatling, everyone else takes one damage and player gets rid of " << returnedArrows << " arrows."<< endl;
    
    //shoot every other player once
    int temp; //store return value of isPlayerDead, doesnt matter
    struct node *ptr; //ptr will traverse list
   
    ptr = start;
    for(int i = 0; i < totalPlayers; i++)
    {
        if(ptr -> data.playerIndex != playerTurn -> data.playerIndex) //if player is not player who rolled the dice
        {
            ptr -> data.currentHealthPoints--; //reduce health by one
            ptr = ptr -> next;
            temp = isPlayerDead(ptr -> data);
        }
        else
            ptr = ptr -> next;
    }
}

void dynamiteDice(struct node *playerTurn)
{
    cout << "Player " << playerTurn -> data.playerIndex << " has 3+ dynamite, they take 1 damage. Current HP: " << playerTurn -> data.currentHealthPoints-1 << endl;
    playerTurn -> data.currentHealthPoints--;
}

void check_ShotDice(struct node *playerTurn, int result[]){  
  //   int choose_direction; //choose 
     
     int player_right =  findFavor(playerTurn->data, playerTurn -> next -> data.playerIndex);
     int player_left = findFavor(playerTurn -> data, playerTurn -> prev -> data.playerIndex );
     int player_second_right =  findFavor(playerTurn -> data, playerTurn -> next -> next-> data.playerIndex);
     int player_second_left =  findFavor(playerTurn -> data, playerTurn -> prev -> prev -> data.playerIndex);
     
    for(int i = 0; i <= 4; i++) 
    {     
        if(gameOver)break;
        if(result[i] == 3) 
        { //shot next player
       // choose_direction = rand() % 2;
            if(player_right < player_left) 
            {
       // if(choose_direction == 0)
                shot_next(playerTurn->data, 1);
                shotFavor(playerTurn-> next -> data, playerTurn->data);
            }
        else
            {
                shot_next(playerTurn-> data, 2);
                shotFavor(playerTurn-> prev -> data, playerTurn->data);
            }
        }
    if(result[i] == 4)
    { //when shot second next player
    
        if(player_right < player_left && player_right < player_second_left && player_right < player_second_right) 
        {
            shot_next(playerTurn -> data, 1);
            shotFavor(playerTurn-> next -> data, playerTurn->data);
        }
    
        else if(player_left < player_right && player_left < player_second_left && player_left < player_second_right)
        {
            shot_next(playerTurn -> data, 2);
            shotFavor(playerTurn-> prev -> data, playerTurn->data);
        }
    
        else if(player_second_right < player_right && player_second_right < player_second_left && player_second_right < player_second_left)  
        {      
            shot_second(playerTurn -> data, 1);
            shotFavor(playerTurn-> next -> next -> data, playerTurn->data);

        }
        else
        {
            shot_second(playerTurn -> data, 2);
            shotFavor(playerTurn-> prev -> prev -> data, playerTurn->data);
        }
      //  choose_direction = rand() % 4;
//        if(choose_direction == 0)
//            shot_next(playerTurn, 1);
//        else if(choose_direction == 1)
//            shot_next(playerTurn, 2);
//        else if(choose_direction == 2)
//            shot_second(playerTurn, 1);
//        else
//            shot_second(playerTurn, 2);
        }
                
    }

}


void shot_next(struct player playerTurn, int direction)
{
    int temp;
    struct node *ptr; 
    ptr = start;
    do{     //find the first player (sheriff)
        ptr = ptr-> next;
        
    }while(ptr-> data.playerIndex != playerTurn.playerIndex);
    
        if(direction == 1){ // when choose right, shot right next player 
       
            cout << "Player " << ptr -> data.playerIndex << " shot Player " << ptr -> next -> data.playerIndex << " for one damage. Player " << ptr -> next -> data.playerIndex << " HP: " << ptr -> next -> data.currentHealthPoints-1 << endl;
            ptr -> next -> data.currentHealthPoints--;
             temp = isPlayerDead(ptr -> next -> data);
        }
        else {      // when choose left, shot left next player 
            
             cout << "Player " << ptr -> data.playerIndex << " shot Player " << ptr -> prev -> data.playerIndex << " for one damage. Player " << ptr -> prev -> data.playerIndex << " HP: " << ptr -> prev -> data.currentHealthPoints-1 << endl;
             ptr -> prev -> data.currentHealthPoints--;
             temp = isPlayerDead(ptr -> prev -> data);
        }
}


void shot_second(struct player playerTurn, int direction)
{
    int temp;
    struct node *ptr; 
    ptr = start;
    do{     //find the first player (sheriff)
        ptr = ptr-> next;   
        
    }while(ptr-> data.playerIndex != playerTurn.playerIndex);
   
        if(direction == 1){  // when choose right, shot right second player 
            
            cout << "Player " << ptr -> data.playerIndex << " shot Player " << ptr -> next -> next -> data.playerIndex << " for one damage. Player " << ptr -> next -> next -> data.playerIndex << " HP: " << ptr -> next -> next -> data.currentHealthPoints-1 << endl;
            ptr -> next -> next -> data.currentHealthPoints--;
            temp = isPlayerDead(ptr -> next -> next -> data);
        }
        else {  // when choose left, shot left second player 
            
            cout << "Player " << ptr -> data.playerIndex << " shot Player " << ptr -> prev -> prev -> data.playerIndex << " for one damage. Player " << ptr -> prev -> prev -> data.playerIndex << " HP: " << ptr -> prev -> prev -> data.currentHealthPoints-1 << endl;
            ptr -> prev -> prev -> data.currentHealthPoints--;
            temp = isPlayerDead(ptr -> prev -> prev -> data);            
        }
}

void gameSetup(int totalPlayers)
{
    if(totalPlayers == 4)//the number of roles available changes depending on the number of players
    {
        roles[1] = 1;
        roles[2] = 2;
        roles[3] = 0;
        reneg = 1;
        outl = 2;
        depu = 0;
    }
    else if(totalPlayers == 5)   
    {
        roles[1] = 1;
        roles[2] = 2;
        roles[3] = 1;
        reneg = 1;
        outl = 2;
        depu = 1;
    }
    else if (totalPlayers == 6)
    {
        roles[1] = 1;
        roles[2] = 3;
        roles[3] = 1;
        reneg = 1;
        outl = 3;
        depu = 1;
    }
    else if (totalPlayers == 7)
    {
        roles[1] = 1;
        roles[2] = 3;
        roles[3] = 2;
        reneg = 1;
        outl = 3;
        depu = 2;
    }
    else
    {
        roles[1] = 2;
        roles[2] = 3;
        roles[3] = 2;
        reneg = 2;
        outl = 3;
        depu = 2;
    }

}

struct player playerInfoSetup(int i)
{
    struct player newPlayer;
    //newPlayer = (struct player)malloc(sizeof(struct player));//allocates space for the players information 
    int HP;
    for(;;)//this will keep looping until the randomm value is within the range, and each number corresponds to a role
    {
        newPlayer.role = rand() % 4;
        if(newPlayer.role == 0 && sher != 0)break;
       
        if(newPlayer.role == 1 && reneg != 0)break;
        
        if(newPlayer.role == 2 && outl != 0)break;
        
        if(newPlayer.role == 3 && depu != 0)break;
    }
    
    HP = rand() % 3 + 7;
    
    switch (newPlayer.role){
        case 0://player is a sheriff 
                newPlayer.maxHealthPoints = HP + 2;//sheriff gets boost to max HP
                newPlayer.currentHealthPoints = HP + 2;
                newPlayer.arrowCount = 0;
                newPlayer.playerIndex = i;
                sher--;//everytime a player is intiialized with a certain role, the number of available slots for that role decreases. This assures that there are not more players than allowed assigned to each role
                break;
        case 1://player is a Renegade
                newPlayer.maxHealthPoints = HP;
                newPlayer.currentHealthPoints = HP;
                newPlayer.arrowCount = 0;
                newPlayer.playerIndex = i;
                reneg--;
                break;
        case 2://player is an outlaw 
                newPlayer.maxHealthPoints = HP;
                newPlayer.currentHealthPoints = HP;
                newPlayer.arrowCount = 0;
                newPlayer.playerIndex = i;
                outl--;
                break;
        case 3://player is a deputy 
                newPlayer.maxHealthPoints = HP;
                newPlayer.currentHealthPoints = HP;
                newPlayer.arrowCount = 0;
                newPlayer.playerIndex = i;
                depu--;
                break;
        default: 
                break;
    }
    
    return newPlayer;
}

int isPlayerDead(struct player damagedPlayer)
{
    if(damagedPlayer.currentHealthPoints <= 0)
    {
        roles[damagedPlayer.role]--;
        cout << "Player " << damagedPlayer.playerIndex << " is dead, their role was "; 
        printRole(damagedPlayer.role);
        cout << endl;
        //deleteFavor(damagedPlayer);
        deleteNode(damagedPlayer);
        isGameOver(); //all game over conditions can only happen when a player dies, so check if game over when a player dies since no need to check else
        return 1;
    }
    else
        return 0;
}

void isGameOver()
{
    cout << endl;
    
    if(roles[0] == 0 && roles[2] == 0 && roles[3] == 0 && gameOver != 1)
    {
        gameOver = 1;
        cout << "GAME OVER: Everyone else died, Renegade(s) win" << endl;
        cout << endl << "Surviving Players:" << endl;
    }
    else if(roles[1] == 0 && roles[2] == 0 && gameOver != 1)
    {
        gameOver = 1;
        cout << "GAME OVER: Outlaws and Renegade(s) died, Sheriff wins" << endl;
        cout << endl << "Surviving Players:" << endl;
    }
    else if(roles[0] == 0 && gameOver != 1)
    {
        gameOver = 1;
        cout << "GAME OVER: Sheriff died, Outlaws win" << endl;
        cout << endl << "Surviving Players:" << endl;
    }
    
    
}

void printRole(int outputRole)
{
    if(outputRole == 0)
        cout << "Sheriff";
    else if(outputRole == 1)
        cout << "Renegade";
    else if(outputRole == 2)
        cout << "Outlaw";
    else
        cout << "Deputy";
}

int checkFreq(int result[], int diceNum)
{
    int i;
    int frequency = 0;
    for(i = 0; i <= 4; i++) //go through dice array
    {
        if(result[i] == diceNum) //if matching increase frequency
            frequency++;
    }
    return frequency;
}

int findFavor(struct player basePlayer, int searchPlayerIndex)
{
    struct favorList *ptr;
    int favor;
    ptr = basePlayer.next;
    
    while(ptr != NULL)
    {
        if(ptr -> favorIndex == searchPlayerIndex)
        {
            favor = ptr -> favorValue;
            break;
        }
        ptr = ptr -> next;
    }
    
    return favor;
}

void sortFavor(struct player basePlayer)
{
    struct favorList *ptr,*head, *currentMax;
    head = basePlayer.next;
    int maxFavor = -10000000;
    int tempIndex, tempFavorVal;
    while (head -> next != NULL)
    {
         //printFavor(basePlayer);
         ptr = head;
         while(ptr!= NULL)//search list for max Favor Value
         {
             if(ptr -> favorValue > maxFavor)//When a favor value is found to be greater than the current greatest, make that one the current greatest
             {
                 currentMax = ptr;
                 maxFavor = ptr -> favorValue;
             }
            ptr = ptr -> next;
         }
         if(currentMax == head)//if the current max is at the head position, continue to next iteration
         {
             head = head -> next;
             maxFavor = -10000000;
         }
         else
         {
             //swap the values of the head and currentMax nodes
             tempFavorVal = currentMax -> favorValue;
             tempIndex = currentMax -> favorIndex;
             currentMax -> favorValue = head -> favorValue;
             currentMax -> favorIndex = head -> favorIndex;
             head -> favorValue = tempFavorVal;
             head -> favorIndex = tempIndex;
             head = head -> next;
             maxFavor = -10000000;
        
         }
    }
}