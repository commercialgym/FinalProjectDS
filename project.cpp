/*
* FILE: project.cpp
* PROJECT: Final Project 25%
* PROGRAMMERS: Valentyn Novosydliuk and Alexia Tu
* DESCRIPTION: This project involves developing an inventory management system for a courier company using hash
    * tables and tree data structures. Parcel data, including destination, weight, and valuation, is loaded and
    * organized into a hash table with 127 buckets. Each bucket contains the root of a binary search tree (BST),
    * where each node represents a parcel, organized by weight. The program supports user interactions through a
    * menu that allows searching, displaying, and analyzing parcel data by country, weight, and valuation. Proper
    * memory management and error handling are emphasized, with dynamic allocation used for string data.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma warning(disable:4996)

#define TABLE_SIZE 127
#define MAX_FLIGHTS 5000 //for load data
#define MIN_FLIGHTS 2000 //^
#define ERROR 1//return code for load data function
#define SUCCESS 0//^
#define VALID_INPUT 1 //used to determine if parse was valid
#define HIGHER 1 //used to check user input for searching by weight
#define LOWER 2 //^
#define SEARCH_HIGH 1
#define SEARCH_LOW 0
#define MAX_WEIGHT 50000
#define MIN_WEIGHT 100
#define MIN_PRICE 10
#define MAX_PRICE 2000

/* Each parcel will have a link to another node in the tree and 3 variables inside */
typedef struct Parcel {
    char* destination;
    int weight;
    float valuation;
} Parcel;

/* Tree node */
typedef struct BSTNode {
    Parcel* parcel;
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;

/* Hash node that will be used to store 127 roots to point to 127 BSTs */
typedef struct HashNode {
    BSTNode* root;
} HashNode;

/* Function prototypes */
void traverseAndAddBST(BSTNode* node, int& totalWeight, float& totalValuation);
HashNode* initializeHashTable(void);
unsigned long computeHash(const char* str);
Parcel* createParcel(const char* destination, int weight, float valuation);
BSTNode* insertBST(BSTNode* root, Parcel* parcel);
int loadData(const char* filename, HashNode* hashTable);
void printParcels(BSTNode* root);
void searchByCountry(const char* country, HashNode* hashTable);
void searchByWeightHelper(BSTNode* root, int weight, int higher);
void searchByWeight(const char* country, int weight, int higher, HashNode* hashTable);
void calculateTotalLoadAndValuation(const char* country, HashNode* hashTable);
void displayCheapestAndMostExpensive(const char* country, HashNode* hashTable);
void displayLightestAndHeaviest(const char* country, HashNode* hashTable);
void findLowestPrice(BSTNode* root, Parcel** cheapestParcel);
void findHighestPrice(BSTNode* root, Parcel** cheapestParcel);
void cleanup(HashNode* hashTable);
void freeBST(BSTNode* root);

int main(void) {
    HashNode* hashTable = initializeHashTable();

    if (loadData("courier.txt", hashTable) == ERROR) {
        printf("Not enough flights provided in the file\n");
        return ERROR;
    }

    int choice = 0;
    char country[21] = { 0 };
    int weight = 0;
    int option = 0;

    while (true) {
        printf("\nMenu:\n");
        printf("1. Enter country name and display all the parcels details\n");
        printf("2. Enter country and weight pair to display parcels higher/lower than weight\n");
        printf("3. Display total parcel load and valuation for the country\n");
        printf("4. Display cheapest and most expensive parcel's details\n");
        printf("5. Display lightest and heaviest parcels for the country\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != VALID_INPUT) {
            printf("Invalid input, please enter a number.\n");
            while (getchar() != '\n'); // Clear invalid input
            continue;
        }

        switch (choice) {
        case 1:
            printf("Enter country name: ");
            if (scanf("%20s", country) != VALID_INPUT) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            searchByCountry(country, hashTable);
            break;
        case 2:
            printf("Enter country name: ");
            if (scanf("%20s", country) != VALID_INPUT) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            printf("Enter weight: ");
            if (scanf("%d", &weight) != VALID_INPUT) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            printf("1. Higher than weight\n2. Lower than weight\n");
            if (scanf("%d", &option) != VALID_INPUT) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            if (option == HIGHER)
                searchByWeight(country, weight, SEARCH_HIGH, hashTable);
            else if (option == LOWER)
                searchByWeight(country, weight, SEARCH_LOW, hashTable);
            else //if they don't enter valid higher or lower weight option, then just restart menu
            {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            break;
        case 3:
            printf("Enter country name: ");
            if (scanf("%20s", country) != VALID_INPUT) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            calculateTotalLoadAndValuation(country, hashTable);
            break;
        case 4:
            printf("Enter country name: ");
            if (scanf("%20s", country) != VALID_INPUT) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            displayCheapestAndMostExpensive(country, hashTable);
            break;
        case 5:
            printf("Enter country name: ");
            if (scanf("%20s", country) != VALID_INPUT) {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clear invalid input
                continue;
            }
            displayLightestAndHeaviest(country, hashTable);
            break;
        case 6:
            cleanup(hashTable);
            free(hashTable);
            return SUCCESS;
        default:
            printf("Invalid choice, try again.\n");
        }
    }
}

/* Initialize the hash table */
//FUNCTION: initializeHashTable()
//PARAMETERS: void
//DESCRIPTION: uses dynamically allocated space to create a hash table, ensures that the root does not have dangling pointer by initializing
// to null. the hash table is dynamically allocated in space but in consecutive blocks so it can still be accessed like an array. The
// size is 127 as the requirements say.
//RETURNS: hashTable - pointer to the beginning of the hashtable
HashNode* initializeHashTable(void) {
    HashNode* hashTable = (HashNode*)malloc(TABLE_SIZE * sizeof(HashNode));
    if (hashTable == NULL) {
        perror("Unable to allocate memory for hash table");
        exit(1);
    }
    for (int i = 0; i < TABLE_SIZE; ++i) {
        hashTable[i].root = NULL;
    }
    return hashTable;
}

/* Hash function using DJB2 algorithm */
//FUNCTION: computeHash()
//PARAMETERS: const char* str - the name of the country
//DESCRIPTION: using the "djb2 function", this function creates unique hash values to be stored into the binary search tree at 
// a specific bucket index of the hash table.
//RETURNS: unsigned long - a generated index to be used in the hash table for the country that was passed to this function
unsigned long computeHash(const char* str) {
    unsigned long hash = 5381;
    int c = 0;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TABLE_SIZE;
}

/* Create a new parcel */
//FUNCTION: createParcel()
//PARAMETERS: const char* destination, int weight, float valuation - values that will be given to the fields of the parcel struct
//DESCRIPTION: dynamically allocates space for the new parcel and also allocates memory for destination dynamically. 
//RETURNS: newParcel - pointer to the new parcel or NULL if the weight and valuation is out of the range
Parcel* createParcel(const char* destination, int weight, float valuation) {
    if (weight > MAX_WEIGHT || weight < MIN_WEIGHT || valuation > MAX_PRICE || valuation < MIN_PRICE) {
        return NULL;
    }
    Parcel* newParcel = (Parcel*)malloc(sizeof(Parcel));
    if (newParcel == NULL) {
        perror("Unable to allocate memory for parcel");
        exit(1);
    }
    newParcel->destination = (char*)malloc(strlen(destination) + 1);
    if (newParcel->destination == NULL) {
        perror("Unable to allocate memory for parcel destination");
        exit(1);
    }
    strcpy(newParcel->destination, destination);
    newParcel->weight = weight; //if gross weight is typically between 100gms and 50 000gms do we need to validate that it's within this range
    newParcel->valuation = valuation; //as above, range is $10 to $2000 ?????????? do we have to error check or is it just for our info
    return newParcel;
}

/* Insert parcel into BST */
//FUNCTION: insertBST()
//PARAMETERS: BSTNode* root - the root of the BST the parcel is about to be inserted into, Parcel* parcel - the new parcel node to be inserted
//DESCRIPTION: dynamically allocates space for the parcel as a BST node, which holds a ptr to the parcel itself, as well as left and right.
// each node in the BST represents a parcel. each node is placed using the parcel's weight. the original root is placed at random, it is 
// assumed (hoped) that the text file is randomized enough so that the lowest or highest weighted node is not the root. the function uses
// recusion to traverse through the tree until the condition of (root == NULL) is met, at which point the bst node will be created to insert it.
//RETURNS: root - first the root of the node that was created, then continuing to return through the recusion until the main recieves the return value
// of the original root of the whole bst
BSTNode* insertBST(BSTNode* root, Parcel* parcel) {
    if (root == NULL) {
        BSTNode* newNode = (BSTNode*)malloc(sizeof(BSTNode));
        if (newNode == NULL) {
            perror("Unable to allocate memory for BST node");
            exit(1);
        }
        newNode->parcel = parcel;
        newNode->left = newNode->right = NULL;
        return newNode;
    }
    if (parcel->weight < root->parcel->weight)
        root->left = insertBST(root->left, parcel);
    else
        root->right = insertBST(root->right, parcel);
    return root;
}

/* Load data from file into hash table */
//FUNCTION: loadData()
//PARAMETERS: const char* filename, HashNode* hashTable - the file name from main to be opened, and the hash table to insert the countries into
//DESCRIPTION: using FILE i/o to read from the courier.txt file. after reading the name of the country as well as it's details, the info is sent
// to the createParcel function to create a new parcel node. the name of the country is then given a hash value from the computeHash function, which 
// value is then used to index the hashTable. the parcel is then inserted at this index of the hashTable with the insertBST() function. the total flights 
// then incremented to ensure that the number of flights does not exceed 5000, as per requirements state. ensures proper error checking for file io
//RETURNS: int - success or error whether there was enough flight data read
int loadData(const char* filename, HashNode* hashTable) {
    FILE* pFile = fopen(filename, "r");
    if (pFile == NULL) {
        perror("Unable to open file\n\n");
        exit(1);
    }
    int totalFlights = 0; //to ensure that the list of names is at least 2000 but does not exceed 5000
    char destination[21];
    int weight = 0;
    float valuation; 
    while ((fscanf(pFile, "%20[^,],%d,%f\n", destination, &weight, &valuation) != EOF) && totalFlights < MAX_FLIGHTS) { 
        Parcel* parcel = createParcel(destination, weight, valuation);
        if (parcel == NULL) { //means there was an issue with weight or valuation
            continue;
        }
        unsigned long index = computeHash(destination);
        hashTable[index].root = insertBST(hashTable[index].root, parcel);
        totalFlights++;
    }
    if (ferror(pFile)) {
        clearerr(pFile);
    }
    if (fclose(pFile) == EOF) {
        printf("Error closing file\n\n");
        clearerr(pFile);
    }
    if (totalFlights < MIN_FLIGHTS)
    {
        return ERROR;
    }
    return SUCCESS;
}

/* Prints parcels info */
//FUNCTION: printParcels()
//PARAMETERS: BSTNode* root - root of a specific index of the hashtable
//DESCRIPTION: prints the bst with in-order traversal. the argument sent to the parameter is actually the root of a specific index of the hash table
// representing a specific country - so this function prints all the parcels of a country. this function is utilized by searchByCountry()
//RETURNS: void
void printParcels(BSTNode* root) {
    if (root != NULL) {
        printParcels(root->left);
        printf("Destination: %s, Weight: %d, Valuation: %.2f\n",
            root->parcel->destination, root->parcel->weight, root->parcel->valuation);
        printParcels(root->right);
    }
}

/* Search parcels by country */
//FUNCTION: searchByCountry()
//PARAMETERS: const char* country, HashNode* hashTable - the country to search and the entire hashtable holding all countries
//DESCRIPTION: this function re-hashes the country name to find the index of the hash table that contains the root of the country's bst.
// then prints the bst of that root by calling the printParcels function
//RETURNS: void
void searchByCountry(const char* country, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    if (root == NULL) {
        printf("No country found for entered country: %s\n", country);
        return;
    }
    printParcels(root);
}

/* Display parcels with weight higher or lower than specified */
//FUNCTION: searchByWeightHelper()
//PARAMETERS: BSTNode* root, int weight, int higher - the country tree to search, the weight of the country being search, and an int variable indicating
// whether to search for higher or lower weighted parcels of the inputted weight
//DESCRIPTION: of the root is null, it means that portion of the tree has been fully searched/is at the leaf of the bst. when deciding whether to 
// print the higher or lower weighted parcels, the integer variable "higher" is used as a boolean value. if the inputted variable is 1 (or true, meaning to
// search for parcels higher than) and the current parcel being searched is greater than the inputted weight, then the details of that current parcel is printed.
// if the boolean "higher" variable is 0 (or false, meaning search for lower), the condition is not false (cancels to true) and the current parcel weight is
// lower than the weight being compared to, the it's details are printed. the function uses in-order traversal, visiting the left subtree, the current, then right.
//RETURNS: void
void searchByWeightHelper(BSTNode* root, int weight, int higher) {
    if (root == NULL) {
        return;
    }
    searchByWeightHelper(root->left, weight, higher);
    if ((higher && root->parcel->weight > weight) ||
        (!higher && root->parcel->weight < weight)) {
        printf("Destination: %s, Weight: %d, Valuation: %.2f\n",
            root->parcel->destination, root->parcel->weight, root->parcel->valuation);
    }
    searchByWeightHelper(root->right, weight, higher);
}

/* Main function to search parcels by weight */
//FUNCTION: searchByWeight()
//PARAMETERS: const char* country, int weight, int higher, HashNode* hashTable
//DESCRIPTION: finds the hash value of the country to find the index of the hash table to send to the searchByWeightHelper() function. it also
// sends the weight of the country being searched, as well as as an int variable called higher. higher is the menu option of 1 or 2, taken from 
// user input in main.
//RETURNS: void
void searchByWeight(const char* country, int weight, int higher, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    searchByWeightHelper(root, weight, higher);
}

/* Calculate total parcel load and valuation for a country */
//FUNCTION: calculateTotalLoadAndValuation()
//PARAMETERS: const char* country, HashNode* hashTable - country being calculated and hashtable to get the country info from
//DESCRIPTION: takes the country being searched for and finds the hash value to access the index of the hashtable which contains the bst for the country.
// it then traverses through the bst to find the total weight and valuation, and finally prints the total two values.
//RETURNS: void
void calculateTotalLoadAndValuation(const char* country, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    int totalWeight = 0;
    float totalValuation = 0.0;

    // traverse the BST and accumulate weights and valuations
    traverseAndAddBST(root, totalWeight, totalValuation);

    printf("Total Load: %d grams, Total Valuation: $%.2f\n", totalWeight, totalValuation);
}

/* Display the cheapest and most expensive parcels */
//FUNCTION: displayCheapestAndMostExpensive()
//PARAMETERS: const char* country, HashNode* hashTable - country being calculated and hashtable to get the country info from
//DESCRIPTION: takes the country being searched for and finds the hash value to access the index of the hashtable which contains the bst for the country.
// first the pointer variables for cheapest and most expensive point to the current root's parcelc, the parcel that these pointers point to is expected to change
// when sent to the findLowestPrice() and findHighestPrice() functions. the root of the hash table is sent to these two functions with the pointers to the
// cheaptest / most expensive, as the functions traverse it will point to the parcel that is true of the condition (of either cheapest or most expensive). it 
// then prints the information of the parcels that these pointers point to.
//RETURNS: void
void displayCheapestAndMostExpensive(const char* country, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    if (root == NULL) {
        printf("No parcels found for country %s\n", country);
        return;
    }
    Parcel* cheapest = root->parcel;
    Parcel* mostExpensive = root->parcel;

    findLowestPrice(root, &cheapest);
    findHighestPrice(root, &mostExpensive);

    printf("Cheapest Parcel - Destination: %s, Weight: %d, Valuation: %.2f\n",
        cheapest->destination, cheapest->weight, cheapest->valuation);
    printf("Most Expensive Parcel - Destination: %s, Weight: %d, Valuation: %.2f\n",
        mostExpensive->destination, mostExpensive->weight, mostExpensive->valuation);
}

//FUNCTION: findLowestPrice()
//PARAMETERS: BSTNode* root, Parcel** cheapestParcel - the bst tree to search and a pointer that will point to the cheapest parcel
//DESCRIPTION: the function uses pre-order traversal. it evaluates the lowest price condition by checking if the current parcel's
// valuation is less than what the cheapestParcel pointer's valuation is. if the current parcel's valuation is cheaper, then the 
// pointer then points to the current parcel. then it traverses the left, then the right sub tree. searching the entire tree to assign
// the cheapestParcel the lowest value.
//RETURNS: void - the pointer does not need to be returned since it's being passed by reference.
void findLowestPrice(BSTNode* root, Parcel** cheapestParcel) {
    if (root == NULL) {
        return;
    }
    //check if the current node's price is lower than the price from the previous root
    if (root->parcel->valuation < (*cheapestParcel)->valuation) {
        *cheapestParcel = root->parcel;
    }

    findLowestPrice(root->left, cheapestParcel); //traverse through the whole bst
    findLowestPrice(root->right, cheapestParcel);
}

//FUNCTION: findHighestPrice()
//PARAMETERS: BSTNode* root, Parcel** expensiveParcel - the bst tree to search and a pointer that will point to the most expensive parcel
//DESCRIPTION: the function uses pre-order traversal. it evaluates the highest price condition by checking if the current parcel's
// valuation is greater than what the expensiveParcel pointer's valuation is. if the current parcel's valuation is more expensive, then the 
// pointer then points to the current parcel. then it traverses the left, then the right sub tree. searching the entire tree to assign
// the expensiveParcel the greatest value.
//RETURNS: void - the pointer does not need to be returned since it's being passed by reference.
void findHighestPrice(BSTNode* root, Parcel** expensiveParcel) {
    if (root == NULL) {
        return;
    }
    //check if the current node's price is lower than the price from the previous root
    if (root->parcel->valuation > (*expensiveParcel)->valuation) {
        *expensiveParcel = root->parcel;
    }

    findHighestPrice(root->left, expensiveParcel); //traverse through the whole bst
    findHighestPrice(root->right, expensiveParcel);
}

/* Display the lightest and heaviest parcels */
//FUNCTION: displayLightestAndHeaviest()
//PARAMETERS: const char* country, HashNode* hashTable - country being calculated and hashtable to get the country info from
//DESCRIPTION: takes the root index and visits the leftmost side of the bst to find the lowest weighted parcel, the visits the rightmost side
// of the bst to find the heaviest. it prints the information of the parcel for these lowets and highest parcels.
//RETURNS: void
void displayLightestAndHeaviest(const char* country, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    BSTNode* rootCopy = hashTable[index].root;
    if (root == NULL) {
        printf("No parcels found for country %s\n", country);
        return;
    }
    while (root->left != NULL) {
        root = root->left;
    }
    printf("Lightest Parcel - Destination: %s, Weight: %d, Valuation: %.2f\n",
        root->parcel->destination, root->parcel->weight, root->parcel->valuation);
    root = rootCopy;
    while (root->right != NULL) {
        root = root->right;
    }

    printf("Heaviest Parcel - Destination: %s, Weight: %d, Valuation: %.2f\n",
        root->parcel->destination, root->parcel->weight, root->parcel->valuation);
}

/* Cleanup memory */
//FUNCTION: cleanup()
//PARAMETERS: HashNode* hashTable
//DESCRIPTION: frees the dynamically allocated space from the hash table, it sends each index of the table to the function freeBST() which
// traverses the entire bst to ensure each node is freed
//RETURNS: void
void cleanup(HashNode* hashTable) {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (hashTable[i].root != NULL) {
            freeBST(hashTable[i].root); //free the BST rooted at this hash table entry
        }
    }
}

//FUNCTION: freeBST()
//PARAMETERS: BSTNode* root - root of the country's bst to be freed
//DESCRIPTION: called from the cleanup function, traverses the entire root of the hashtable index. using post order traversal. visits the left then right
// subtree before deleting the node, it ensures the dynamically allocated space from the parcel node (destination) and bst node (parcel) is also freed, 
// before freeing the actual node of the hash table itself.
//RETURNS: void
void freeBST(BSTNode* root) {
    if (root != NULL) {
        freeBST(root->left);
        freeBST(root->right);
        free(root->parcel->destination);
        free(root->parcel);
        free(root);
    }
}


//FUNCTION: traverseAndAddBST()
//PARAMETERS: BSTNode* node, int& totalWeight, float& totalValuation - the root of the bst being searched, a variable for total weight and valuation
//DESCRIPTION: searches the tree with in-order traversal. it visits every node of the bst, accesses the parcel node, and adds the value of the weight
// and valuation of that parcel to the total variables. used by the calculateTotalLoadAndValuation() function
//RETURNS: void - variables are passed by reference and directly altered
void traverseAndAddBST(BSTNode* node, int& totalWeight, float& totalValuation) {
    if (node == NULL) {
        return;
    }
    //traverse the left subtree
    traverseAndAddBST(node->left, totalWeight, totalValuation);
    //visit the current node
    totalWeight += node->parcel->weight;
    totalValuation += node->parcel->valuation;
    //traverse the right subtree
    traverseAndAddBST(node->right, totalWeight, totalValuation);
}