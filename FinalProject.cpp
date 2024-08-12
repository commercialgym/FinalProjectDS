/*
* FILE: project.cpp
* PROJECT: Final Project 25%
* PROGRAMMERS: Valentyn N and Alexia Tu
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
#define MAX_FLIGHTS 5000
#define VALID_INPUT 1 //used to determine if parse was valid
#define HIGHER 1
#define LOWER 2

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

/* hash node that will be used to store 127 roots to point to 127 BSTs */
typedef struct HashNode {
    BSTNode* root; 
} HashNode;

/* Function prototypes */
void traverseBST(BSTNode* node, int& totalWeight, float& totalValuation);
HashNode* initializeHashTable(void);
unsigned long computeHash(const char* str);
Parcel* createParcel(const char* destination, int weight, float valuation);
BSTNode* insertBST(BSTNode* root, Parcel* parcel);
void loadData(const char* filename, HashNode* hashTable);
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

int main(void) {
    HashNode* hashTable = initializeHashTable();

    loadData("couriers.txt", hashTable);

    int choice;
    char country[21];
    int weight;
    int option;

    while (true) {
        printf("\nMenu:\n");
        printf("1. Enter country name and display all the parcels details\n");
        printf("2. Enter country and weight pair to display parcels higher/lower than weight\n");
        printf("3. Display total parcel load and valuation for the country\n");
        printf("4. Display cheapest and most expensive parcel’s details\n");
        printf("5. Display lightest and heaviest parcel for the country\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != VALID_INPUT) {
            printf("Invalid input, please enter a number.\n");
            while (getchar() != '\n'); // Clear invalid input VALENTYN - how does this work?
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
                searchByWeight(country, weight, 1, hashTable);
            else if (option == LOWER)
                searchByWeight(country, weight, 0, hashTable);
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
            return 0;
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
    unsigned long hash = 5381; //magic num
    int c = 0;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; //magic num
    }
    return hash % TABLE_SIZE;
}

/* Create a new parcel */
//FUNCTION: createParcel()
//PARAMETERS: const char* destination, int weight, float valuation - values that will be given to the fields of the parcel struct
//DESCRIPTION: dynamically allocates space for the new parcel and also allocates memory for destination dynamically. 
//RETURNS: 
Parcel* createParcel(const char* destination, int weight, float valuation) {
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
//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: ?????????? could make more modular and create an initialize data node function
// each node in the BST represents a parcel. each node is placed using the parcel's weight
//RETURNS: 
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
//FUNCTION: 
//PARAMETERS: 
//DESCRIPTION: it's play to have more than 2000 names but not more than 5000, hence the while condition including "totalFLights < MAX_FLIGHTS"
//RETURNS: 
void loadData(const char* filename, HashNode* hashTable) {
    FILE* pFile = fopen(filename, "r");
    if (pFile == NULL) {
        perror("Unable to open file\n\n");
        exit(1);
    }
    int totalFlights = 0; //to ensure that the list of names is at least 2000 but does not exceed 5000
    char destination[21];
    int weight;
    float valuation; //?????????????????does it account for empty /  new lines
    while ((fscanf(pFile, "%20[^,],%d,%f\n", destination, &weight, &valuation) != EOF) && totalFlights < MAX_FLIGHTS) { //EOF or NULL?
        Parcel* parcel = createParcel(destination, weight, valuation);
        unsigned long index = computeHash(destination);
        hashTable[index].root = insertBST(hashTable[index].root, parcel);
        totalFlights++; 
    }
    if (ferror(pFile)){
        clearerr(pFile);
    }
    if (fclose(pFile) == EOF) {
        printf("Error closing file\n\n");
        clearerr(pFile);
    }
    
}

/* Prints parcels info */
//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: prints the bst with in-order traversal
//RETURNS: 
void printParcels(BSTNode* root) {
    if (root != NULL) {
        printParcels(root->left);
        printf("Destination: %s, Weight: %d, Valuation: %.2f\n",
            root->parcel->destination, root->parcel->weight, root->parcel->valuation);
        printParcels(root->right);
    }
}

/* Search parcels by country */
//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: re-hashes country name to find the index of the hash table that contains the root of the country's bst.
// then prints the bst of that root by calling the printParcels function
//RETURNS: 
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
//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: 
//RETURNS: 
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
//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: 
//RETURNS: 
void searchByWeight(const char* country, int weight, int higher, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    searchByWeightHelper(root, weight, higher);
}

/* Calculate total parcel load and valuation for a country */
//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: 
//RETURNS: 
void calculateTotalLoadAndValuation(const char* country, HashNode* hashTable) {
    unsigned long index = computeHash(country);
    BSTNode* root = hashTable[index].root;
    int totalWeight = 0;
    float totalValuation = 0.0;

    // Traverse the BST and accumulate weights and valuations
    traverseBST(root, totalWeight, totalValuation);

    printf("Total Load: %d grams, Total Valuation: $%.2f\n", totalWeight, totalValuation);
}

/* Display the cheapest and most expensive parcels */
//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: 
//RETURNS: 
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

//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: 
//RETURNS: 
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

//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: 
//RETURNS: 
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
//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: 
//RETURNS: 
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
//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: 
//RETURNS: 
void cleanup(HashNode* hashTable) {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        BSTNode* root = hashTable[i].root;
        while (root != NULL) {
            BSTNode* temp = root;
            root = root->right;
            if (temp->parcel != NULL){ //did this get rid of warning?
                free(temp->parcel->destination);
                free(temp->parcel);
            }
            free(temp);
        }
    }
}

//FUNCTION: 
//PARAMETERS:
//DESCRIPTION: 
//RETURNS: 
void traverseBST(BSTNode* node, int& totalWeight, float& totalValuation) {
    if (node == NULL) {
        return;
    }
    // Traverse the left subtree
    traverseBST(node->left, totalWeight, totalValuation);
    // Visit the current node
    totalWeight += node->parcel->weight;
    totalValuation += node->parcel->valuation;
    // Traverse the right subtree
    traverseBST(node->right, totalWeight, totalValuation);
}