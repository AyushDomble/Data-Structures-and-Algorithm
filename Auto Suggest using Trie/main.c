#include <stdio.h>    // For standard input/output functions like printf, scanf
#include <stdlib.h>   // For memory allocation functions like malloc, free, and exit
#include <stdbool.h>  // For using the boolean data type (true, false)
#include <string.h>   // For string manipulation functions like strcpy, strcmp, strlen

// --- MACRO DEFINITIONS ---

#define ALPHABET_SIZE 26       // The number of letters in the English alphabet
#define MAX_WORD_LEN 100       // Maximum length of a word that can be processed
#define DICTIONARY_FILE "Dictionary.txt" // Filename for the dictionary
#define MAX_SESSION_WORDS 1000 // Maximum number of words that can be added/deleted in one session
#define STATS_FILE "SearchStats.txt" // Filename for storing search frequency statistics

// ANSI color macros for styling the console output
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define ORANGE "\x1b[93m"
#define RESET "\x1b[0m" // Resets text color to default
#define WHITE "\x1b[37m"
#define GREY "\x1b[90m"
#define BOLDYELLOW "\x1b[1;33m"
#define BOLDCYAN "\x1b[1;36m"
#define BOLDRED "\x1b[1;31m"

// --- DATA STRUCTURES ---

// Structure for a single node in the Trie data structure
typedef struct TrieNode
{
    struct TrieNode *children[ALPHABET_SIZE]; // Array of pointers to child nodes, one for each letter
    bool isEndOfWord;                         // Flag to mark if a node represents the end of a complete word
} TrieNode;

// Structure to track the frequency of searched words
typedef struct
{
    char word[MAX_WORD_LEN]; // The word/prefix that was searched
    int frequency;           // How many times it has been searched
} WordFrequency;

// Global array to store word search frequencies for the current session
WordFrequency wordFreqList[MAX_SESSION_WORDS];
int wordFreqCount = 0; // Counter for the number of unique words tracked in wordFreqList

// --- TRIE FUNCTIONS ---

/**
 * @brief Creates and initializes a new TrieNode.
 * @return A pointer to the newly allocated TrieNode, or NULL if allocation fails.
 */
TrieNode *createNode()
{
    // Allocate memory for a new node
    TrieNode *newNode = (TrieNode *)malloc(sizeof(TrieNode));
    if (newNode)
    {
        // Mark that this node is not the end of a word by default
        newNode->isEndOfWord = false;
        // Initialize all children pointers to NULL
        for (int i = 0; i < ALPHABET_SIZE; i++)
            newNode->children[i] = NULL;
    }
    return newNode;
}

/**
 * @brief Converts a given string to lowercase.
 * @param str The string to be converted.
 */
void toLowerCase(char *str)
{
    // Iterate through the string until the null terminator is found
    for (int i = 0; str[i]; i++)
        // If the character is an uppercase letter, convert it to lowercase
        if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] += ('a' - 'A'); // Add the offset between 'a' and 'A'
}

/**
 * @brief Inserts a word into the Trie.
 * @param root The root node of the Trie.
 * @param word The word to insert.
 */
void insert(TrieNode *root, const char *word)
{
    TrieNode *node = root;
    // Iterate through each character of the word
    while (*word)
    {
        // Calculate the index (0-25) for the character
        int index = *word - 'a';
        // If the character is not a valid lowercase letter, stop
        if (index < 0 || index >= ALPHABET_SIZE)
            return;
        // If the child node for this character doesn't exist, create it
        if (!node->children[index])
            node->children[index] = createNode();
        // Move to the child node
        node = node->children[index];
        // Move to the next character in the word
        word++;
    }
    // After inserting all characters, mark the final node as the end of a word
    node->isEndOfWord = true;
}

/**
 * @brief Searches for a prefix in the Trie and returns the node where the prefix ends.
 * @param root The root node of the Trie.
 * @param prefix The prefix to search for.
 * @return The TrieNode at the end of the prefix, or NULL if the prefix is not found.
 */
TrieNode *searchPrefix(TrieNode *root, const char *prefix)
{
    TrieNode *node = root;
    // Iterate through each character of the prefix
    while (*prefix)
    {
        int index = *prefix - 'a';
        // If the character is not a valid lowercase letter, the prefix doesn't exist
        if (index < 0 || index >= ALPHABET_SIZE)
            return NULL;
        // If there's no path for this character, the prefix doesn't exist
        if (!node->children[index])
            return NULL;
        // Move to the next node in the path
        node = node->children[index];
        prefix++;
    }
    // Return the node where the prefix ends
    return node;
}

/**
 * @brief Recursively collects and prints all words starting from a given Trie node.
 * This function performs a Depth-First Search (DFS).
 * @param node The starting node for collection.
 * @param buffer A character buffer to build the current word.
 * @param depth The current depth in the Trie (length of the word in the buffer).
 */
void collectWords(TrieNode *node, char *buffer, int depth)
{
    // If the current node marks the end of a word, print the word
    if (node->isEndOfWord)
    {
        buffer[depth] = '\0'; // Null-terminate the string
        printf(CYAN " - %s\n" RESET, buffer);
    }
    // Recur for all children of the current node
    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        if (node->children[i])
        {
            // Add the character to the buffer
            buffer[depth] = i + 'a';
            // Recursively call for the child node, increasing the depth
            collectWords(node->children[i], buffer, depth + 1);
        }
    }
}

/**
 * @brief Updates the frequency count for a searched word/prefix.
 * @param word The word whose search frequency needs to be updated.
 */
void updateFrequency(const char *word)
{
    // Check if the word is already in our frequency list
    for (int i = 0; i < wordFreqCount; i++)
    {
        if (strcmp(wordFreqList[i].word, word) == 0)
        {
            // If found, increment its frequency and return
            wordFreqList[i].frequency++;
            return;
        }
    }
    // If the word is not in the list, add it as a new entry
    strcpy(wordFreqList[wordFreqCount].word, word);
    wordFreqList[wordFreqCount].frequency = 1;
    wordFreqCount++; // Increment the count of unique searched words
}

/**
 * @brief Provides auto-suggestions for a given prefix.
 * @param root The root node of the Trie.
 * @param prefix The prefix to find suggestions for.
 */
void autoSuggest(TrieNode *root, const char *prefix)
{
    // Find the node where the prefix ends
    TrieNode *node = searchPrefix(root, prefix);
    if (!node)
    {
        printf(BOLDRED "No suggestions found.\n" RESET);
        return;
    }

    // Track the search frequency for this prefix
    updateFrequency(prefix);

    char buffer[MAX_WORD_LEN];
    strcpy(buffer, prefix); // Start the buffer with the prefix
    printf(GREEN "Suggestions:\n" RESET);
    // Collect all words that start from the prefix node
    collectWords(node, buffer, strlen(prefix));
}

// --- FILE I/O AND UTILITY FUNCTIONS ---

/**
 * @brief Loads words from the dictionary file into the Trie.
 * @param root The root node of the Trie.
 */
void loadDictionary(TrieNode *root)
{
    FILE *file = fopen(DICTIONARY_FILE, "r"); // Open the file in read mode
    if (!file)
    {
        // If the file doesn't exist, show a warning but continue
        printf(ORANGE "Warning: Dictionary file not found. Proceeding with an empty trie.\n" RESET);
        return;
    }

    char word[MAX_WORD_LEN];
    // Read each line (word) from the file
    while (fgets(word, MAX_WORD_LEN, file))
    {
        // Remove trailing newline or carriage return characters
        word[strcspn(word, "\r\n")] = 0;
        toLowerCase(word); // Convert the word to lowercase
        insert(root, word);  // Insert the word into the Trie
    }
    fclose(file); // Close the file
    printf(GREEN "Dictionary loaded successfully!\n" RESET);
}

/**
 * @brief Appends a new word to the dictionary file.
 * @param word The word to save.
 */
void saveWordToFile(const char *word)
{
    FILE *file = fopen(DICTIONARY_FILE, "a"); // Open the file in append mode
    if (!file)
    {
        printf(BOLDRED "Error opening dictionary file!\n" RESET);
        return;
    }
    fprintf(file, "%s\n", word); // Write the word followed by a newline
    fclose(file);                // Close the file
}

/**
 * @brief Checks if a TrieNode has any children.
 * @param node The node to check.
 * @return true if the node has no children, false otherwise.
 */
bool isEmpty(TrieNode *node)
{
    for (int i = 0; i < ALPHABET_SIZE; i++)
        if (node->children[i])
            return false; // Found a child, so it's not empty
    return true;
}

/**
 * @brief A recursive helper function to delete a word from the Trie.
 * @param node The current node in the traversal.
 * @param word The remaining part of the word to delete.
 * @return true if the parent node should delete the reference to this node.
 */
bool deleteWordHelper(TrieNode *node, const char *word)
{
    // If we haven't reached the end of the word
    if (*word)
    {
        int index = *word - 'a';
        // If the path doesn't exist, the word isn't in the trie
        if (index < 0 || index >= ALPHABET_SIZE || !node->children[index])
            return false;

        // Recur for the next character
        bool shouldDeleteChild = deleteWordHelper(node->children[index], word + 1);

        // If the recursive call indicates the child node should be deleted
        if (shouldDeleteChild)
        {
            free(node->children[index]); // Free the child node's memory
            node->children[index] = NULL;
            // Return true if this node is not an end of another word and has no other children
            return !node->isEndOfWord && isEmpty(node);
        }
    }
    // If we have reached the end of the word
    else if (node->isEndOfWord)
    {
        node->isEndOfWord = false; // Unmark it as the end of a word
        // If this node has no children, it's safe to delete
        return isEmpty(node);
    }
    return false; // Default case
}

/**
 * @brief Searches for a complete word in the Trie.
 * @param root The root of the Trie.
 * @param word The word to search for.
 * @return true if the word exists, false otherwise.
 */
bool searchWord(TrieNode *root, const char *word)
{
    TrieNode *node = root;
    while (*word)
    {
        int index = *word - 'a';
        if (index < 0 || index >= ALPHABET_SIZE || !node->children[index])
            return false; // Path does not exist
        node = node->children[index];
        word++;
    }
    // The word exists only if the final node is not null AND it's marked as the end of a word
    return node && node->isEndOfWord;
}

/**
 * @brief Deletes a word from the Trie and the dictionary file.
 * @param root The root of the Trie.
 * @param word The word to delete.
 */
void deleteWord(TrieNode *root, const char *word)
{
    // First, check if the word actually exists in the Trie
    if (searchWord(root, word))
    {
        deleteWordHelper(root, word); // Delete it from the Trie data structure
        printf(GREEN "Word deleted successfully from Trie.\n" RESET);

        // Now, remove the word from the dictionary file by rewriting it
        FILE *file = fopen(DICTIONARY_FILE, "r");
        FILE *temp = fopen("temp.txt", "w"); // Create a temporary file
        char line[MAX_WORD_LEN];
        // Read each line from the original dictionary
        while (fgets(line, sizeof(line), file))
        {
            line[strcspn(line, "\r\n")] = 0; // Remove newline
            // If the line is not the word to be deleted, write it to the temp file
            if (strcmp(line, word) != 0)
                fprintf(temp, "%s\n", line);
        }
        fclose(file);
        fclose(temp);
        remove(DICTIONARY_FILE);              // Delete the old dictionary
        rename("temp.txt", DICTIONARY_FILE); // Rename temp file to the original name
    }
    else
    {
        printf(BOLDRED "Word not found in Trie.\n" RESET);
    }
}

/**
 * @brief Displays all words currently in the dictionary (Trie).
 * @param root The root node of the Trie.
 */
void displayAllWords(TrieNode *root)
{
    char buffer[MAX_WORD_LEN];
    printf(ORANGE "All words in dictionary:\n" RESET);
    collectWords(root, buffer, 0); // Use the recursive collect function from the root
}

/**
 * @brief Recursively finds the shortest and longest words in the Trie.
 * @param node Current node.
 * @param buffer Buffer to build words.
 * @param depth Current depth in Trie.
 * @param shortestWords Array to store shortest words.
 * @param shortestLen Pointer to the length of the current shortest word.
 * @param shortestCount Pointer to the count of shortest words found.
 * @param longestWords Array to store longest words.
 * @param longestLen Pointer to the length of the current longest word.
 * @param longestCount Pointer to the count of longest words found.
 */
void findShortestLongestWords(
    TrieNode *node, char *buffer, int depth,
    char **shortestWords, int *shortestLen, int *shortestCount,
    char **longestWords, int *longestLen, int *longestCount)
{

    if (node->isEndOfWord)
    {
        buffer[depth] = '\0';
        int len = strlen(buffer);

        // Check for shortest word
        if (*shortestLen == -1 || len < *shortestLen)
        {
            *shortestLen = len;        // Found a new shortest length
            *shortestCount = 0;        // Reset the count
            strcpy(shortestWords[(*shortestCount)++], buffer); // Store the new shortest word
        }
        else if (len == *shortestLen)
        {
            strcpy(shortestWords[(*shortestCount)++], buffer); // Found another word of the same shortest length
        }

        // Check for longest word
        if (*longestLen == -1 || len > *longestLen)
        {
            *longestLen = len;       // Found a new longest length
            *longestCount = 0;       // Reset the count
            strcpy(longestWords[(*longestCount)++], buffer); // Store the new longest word
        }
        else if (len == *longestLen)
        {
            strcpy(longestWords[(*longestCount)++], buffer); // Found another word of the same longest length
        }
    }

    // Recur for all children
    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        if (node->children[i])
        {
            buffer[depth] = i + 'a';
            findShortestLongestWords(
                node->children[i], buffer, depth + 1,
                shortestWords, shortestLen, shortestCount,
                longestWords, longestLen, longestCount);
        }
    }
}

/**
 * @brief Displays the shortest and longest word(s) in the dictionary.
 * @param root The root of the Trie.
 */
void showShortestLongestWord(TrieNode *root)
{
    char buffer[MAX_WORD_LEN];
    // Allocate memory for arrays to hold potentially multiple shortest/longest words
    char *shortestWords[100], *longestWords[100];
    for (int i = 0; i < 100; i++)
    {
        shortestWords[i] = malloc(MAX_WORD_LEN);
        longestWords[i] = malloc(MAX_WORD_LEN);
    }

    int shortestLen = -1, longestLen = -1; // Initialize lengths to -1 (not found yet)
    int shortestCount = 0, longestCount = 0;

    // Call the recursive helper function to find the words
    findShortestLongestWords(
        root, buffer, 0,
        shortestWords, &shortestLen, &shortestCount,
        longestWords, &longestLen, &longestCount);

    // Display the results
    if (shortestCount > 0 && longestCount > 0)
    {
        printf(MAGENTA "Shortest word(s):\n" RESET);
        for (int i = 0; i < shortestCount; i++)
            printf(CYAN " - %s\n" RESET, shortestWords[i]);

        printf(MAGENTA "Longest word(s):\n" RESET);
        for (int i = 0; i < longestCount; i++)
            printf(CYAN " - %s\n" RESET, longestWords[i]);
    }
    else
    {
        printf(BOLDRED "Trie is empty.\n" RESET);
    }

    // Free the allocated memory
    for (int i = 0; i < 100; i++)
    {
        free(shortestWords[i]);
        free(longestWords[i]);
    }
}

/**
 * @brief Shows all words that have been added during the current program session.
 * @param sessionWords Array of words added this session.
 * @param sessionWordCount Count of words in the array.
 */
void showRecentlyAdded(char **sessionWords, int sessionWordCount)
{
    if (sessionWordCount == 0)
    {
        printf(YELLOW "No words added during this session.\n" RESET);
        return;
    }
    printf(BOLDYELLOW "Recently Added Words (This Session):\n" RESET);
    for (int i = 0; i < sessionWordCount; i++)
    {
        printf(CYAN " - %s\n" RESET, sessionWords[i]);
    }
}

/**
 * @brief Shows all words that have been deleted during the current program session.
 * @param deletedSessionWords Array of words deleted this session.
 * @param deletedSessionWordCount Count of words in the array.
 */
void showRecentlyDeleted(char **deletedSessionWords, int deletedSessionWordCount)
{
    if (deletedSessionWordCount == 0)
    {
        printf(YELLOW "No words deleted during this session.\n" RESET);
        return;
    }
    printf(BOLDRED "Recently Deleted Words (This Session):\n" RESET);
    for (int i = 0; i < deletedSessionWordCount; i++)
    {
        printf(RED " - %s\n" RESET, deletedSessionWords[i]);
    }
}

/**
 * @brief Displays the most frequently searched prefixes from the stats.
 */
void showMostFrequentSearches()
{
    if (wordFreqCount == 0)
    {
        printf(YELLOW "No search history found.\n" RESET);
        return;
    }
    printf(BOLDYELLOW "Most Frequently Searched Words:\n" RESET);
    // Note: This just lists them; for a true "most frequent", it would need to be sorted.
    for (int i = 0; i < wordFreqCount; i++)
    {
        printf(CYAN " - %s (%d times)\n" RESET, wordFreqList[i].word, wordFreqList[i].frequency);
    }
}

/**
 * @brief Loads search statistics from the stats file at the start of the program.
 */
void loadSearchStats()
{
    FILE *file = fopen(STATS_FILE, "r");
    if (!file)
        return; // If file doesn't exist, just return silently

    char word[MAX_WORD_LEN];
    int freq;

    // Read word and frequency pairs from the file
    while (fscanf(file, "%s %d", word, &freq) == 2)
    {
        strcpy(wordFreqList[wordFreqCount].word, word);
        wordFreqList[wordFreqCount].frequency = freq;
        wordFreqCount++;
    }

    fclose(file);
}

/**
 * @brief Saves the current search statistics to the stats file before exiting.
 */
void saveSearchStats()
{
    FILE *file = fopen(STATS_FILE, "w"); // Open in write mode to overwrite old stats
    if (!file)
        return;

    // Write each tracked word and its frequency to the file
    for (int i = 0; i < wordFreqCount; i++)
    {
        fprintf(file, "%s %d\n", wordFreqList[i].word, wordFreqList[i].frequency);
    }

    fclose(file);
}

// --- MAIN FUNCTION ---

int main()
{
    TrieNode *root = createNode(); // Create the root of the Trie
    loadDictionary(root);          // Load existing words from the file
    loadSearchStats();             // Load previous search statistics

    int choice;
    char word[MAX_WORD_LEN];

    // Arrays to keep track of words added/deleted in the current session
    char *sessionWords[MAX_SESSION_WORDS];
    int sessionWordCount = 0;

    char *deletedSessionWords[MAX_SESSION_WORDS];
    int deletedSessionWordCount = 0;

    // Main program loop
    while (1)
    {
        // Display the menu
        printf("\n" BOLDCYAN "--- Auto-Suggest System ---\n" RESET);
        printf("\x1b[38;5;208m"); // Orange color for menu options
        printf("1. Add a new word\n");
        printf("2. Search by prefix (Auto-suggestions)\n");
        printf("3. Display all words\n");
        printf("4. Show recently added words\n");
        printf("5. Show shortest & longest word\n");
        printf("6. Delete a word\n");
        printf("7. Show recently deleted words\n");
        printf("8. Undo last deleted word\n");
        printf("9. Show most frequently searched words\n");
        printf("10. Exit\n");
        printf(RESET "Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume the newline character left by scanf

        switch (choice)
        {
        case 1: // Add a new word
            printf(GREY "Enter word to add: " RESET);
            scanf("%s", word);
            toLowerCase(word);
            insert(root, word);
            saveWordToFile(word); // Persist the new word to the dictionary file
            // Add to session history
            sessionWords[sessionWordCount] = malloc(strlen(word) + 1);
            strcpy(sessionWords[sessionWordCount++], word);
            printf(GREEN "Word added successfully!\n" RESET);
            break;
        case 2: // Search by prefix
            printf(GREY "Enter prefix: " RESET);
            scanf("%s", word);
            toLowerCase(word);
            autoSuggest(root, word);
            break;
        case 3: // Display all words
            displayAllWords(root);
            break;
        case 4: // Show recently added
            showRecentlyAdded(sessionWords, sessionWordCount);
            break;
        case 5: // Show shortest & longest
            showShortestLongestWord(root);
            break;
        case 6: // Delete a word
            printf(GREY "Enter word to delete: " RESET);
            scanf("%s", word);
            toLowerCase(word);

            if (searchWord(root, word))
            {
                // Confirmation prompt before deleting
                char confirm;
                printf(ORANGE "Are you sure you want to delete \"%s\"? (y/n): " RESET, word);
                getchar(); // Consume previous newline
                scanf("%c", &confirm);
                if (confirm == 'y' || confirm == 'Y')
                {
                    deleteWord(root, word);
                    // Remove from "recently added" list if it was there
                    for (int i = 0; i < sessionWordCount; i++)
                    {
                        if (strcmp(sessionWords[i], word) == 0)
                        {
                            free(sessionWords[i]);
                            // Shift elements to fill the gap
                            for (int j = i; j < sessionWordCount - 1; j++)
                            {
                                sessionWords[j] = sessionWords[j + 1];
                            }
                            sessionWords[--sessionWordCount] = NULL;
                            break;
                        }
                    }
                    // Add to "recently deleted" list for the undo feature
                    deletedSessionWords[deletedSessionWordCount] = malloc(strlen(word) + 1);
                    strcpy(deletedSessionWords[deletedSessionWordCount++], word);
                }
                else
                {
                    printf(YELLOW "Deletion cancelled.\n" RESET);
                }
            }
            else
            {
                printf(BOLDRED "Word not found in Trie.\n" RESET);
            }
            break;
        case 7: // Show recently deleted
            showRecentlyDeleted(deletedSessionWords, deletedSessionWordCount);
            break;
        case 8: // Undo last deletion
            if (deletedSessionWordCount == 0)
            {
                printf(YELLOW "No deleted words to undo.\n" RESET);
            }
            else
            {
                // Get the last deleted word
                char *wordToRestore = deletedSessionWords[--deletedSessionWordCount];
                insert(root, wordToRestore);      // Add back to Trie
                saveWordToFile(wordToRestore);  // Add back to file
                // Add back to "recently added" list
                sessionWords[sessionWordCount] = malloc(strlen(wordToRestore) + 1);
                strcpy(sessionWords[sessionWordCount++], wordToRestore);
                printf(GREEN "Successfully restored \"%s\" to Trie and Dictionary.\n" RESET, wordToRestore);
                free(deletedSessionWords[deletedSessionWordCount]); // Free pointer from deleted list
                deletedSessionWords[deletedSessionWordCount] = NULL;
            }
            break;
        case 9: // Show frequent searches
            showMostFrequentSearches();
            break;
        case 10: // Exit
            printf(BOLDYELLOW "PROGRAM EXITED SUCCESSFULLY.\n" RESET);
            saveSearchStats(); // Save search history before exiting
            // Clean up dynamically allocated memory
            for (int i = 0; i < sessionWordCount; i++)
                free(sessionWords[i]);
            for (int i = 0; i < deletedSessionWordCount; i++)
                free(deletedSessionWords[i]);
            // (Note: A full Trie cleanup function would free all nodes, but is omitted here for simplicity)
            return 0;
        default:
            printf(BOLDRED "Invalid choice! Please try again.\n" RESET);
        }
    }
}

