//omer malik kalembasi  
//150180112

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#define NUM_CUSTOMERS 3
#define NUM_PRODUCTS 5

typedef struct {
    int customerID;
    double customerBalance;
    int orderedArr[NUM_PRODUCTS];
    int purchasedArr[NUM_PRODUCTS];
    int order[NUM_PRODUCTS];
} Customer;

typedef struct {
    int productID;
    double productPrice;
    int productQuantity;
} Product;

Customer customers[NUM_CUSTOMERS]; //pointer to shared memory for customers
Product products[NUM_PRODUCTS]; //pointer to shared memory for products
pthread_mutex_t lock; //mutex lock for synchronization

//initialize products with random prices and quantities
void initialize_products() {
    srand(time(NULL));
    for (int index = 0; index < NUM_PRODUCTS; index++) {
        double price_product = (double)rand()/(double)(RAND_MAX/200); //price of the product (between 1 – 200$).
        int quantity_product = 1 + rand() % 10;  //quantity of the products in stock (1-10 for each).
        products[index].productID = index + 1;
        products[index].productPrice = price_product;
        products[index].productQuantity = quantity_product;
    }
}

//initialize customers with random balances and empty order and purchase lists
void initialize_customers() {
    srand(time(NULL));
    for (int index = 0; index < NUM_CUSTOMERS; index++) {
        double balance_customer = (double)rand()/(double)(RAND_MAX/200); //customer’s available money in their account (between 0 – 200$)
        customers[index].customerID = index + 1;
        customers[index].customerBalance = balance_customer;
        for (int productIndex = 0; productIndex < NUM_PRODUCTS; productIndex++) {
            customers[index].orderedArr[productIndex] = 0;
            customers[index].purchasedArr[productIndex] = 0;
        }
    }
}

//order a product for a customer
void order_product(Customer *customer, Product *product, int quantity) {
    pthread_mutex_lock(&lock);  //locks the product, prevents multiple processes from accessing simultaneously
    if (product->productQuantity >= quantity) {
        double balance = customer->customerBalance;
        if (balance >= product->productPrice*quantity) {
            product->productQuantity -= quantity;
            customer->customerBalance -= product->productPrice*quantity;
            customer->orderedArr[product->productID-1] += quantity;
            customer->purchasedArr[product->productID-1] += quantity;
            printf("- Customer%d(%d,%d) success! Paid $%.2lf for each.\n",
                   customer->customerID, product->productID, quantity, product->productPrice);
        } else {
            printf("- Customer%d(%d,%d) fail! Insufficient funds.\n",
                   customer->customerID, product->productID, quantity);
        }
    } else {
        printf("- Customer%d(%d,%d) fail! Only %d left in stock.\n",
               customer->customerID, product->productID, quantity, product->productQuantity);
    }
    pthread_mutex_unlock(&lock);    //unlocks the product
}

//function to order multiple products
void order_products(Customer *customer, int items[NUM_PRODUCTS]) {
    printf("\nCustomer %d has ordered:\n", customer->customerID);
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        if (items[i] > 0) {
            order_product(customer, &products[i], items[i]);
        }
    }
}

//function to manage customer threads
void* customer_thread(void* arg) {  
    int customer_index = *(int*)arg;    //get the customer index from the argument
    order_products(&customers[customer_index], customers[customer_index].order);    //order products for this customer
    pthread_exit(NULL);
}

//print the initial inventory
void print_initial_inventory() {
    printf("\nInitial inventory:\n");
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        printf("Product %d: $%.2lf, %d units\n", products[i].productID, products[i].productPrice, products[i].productQuantity);
    }
}

//print the initial customer balances
void print_initial_balances() {
    printf("\nInitial customer balances:\n");
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        printf("Customer %d: $%.2lf\n", customers[i].customerID, customers[i].customerBalance);
    }
}

//print the latest customer balances
void print_latest_balances() {
    printf("\nLatest customer balances:\n");
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        printf("Customer %d: $%.2lf\n", customers[i].customerID, customers[i].customerBalance);
    }
}

int main() {

    clock_t start, end;
    double cpu_time_used;

    start = clock();    //measure time elaspsed

    initialize_products();
    initialize_customers();
    print_initial_inventory();
    print_initial_balances();

    int sample_order[NUM_PRODUCTS] = {2, 1, 3, 1, 1}; //define a sample order to be ordered by each customer

    //assign the sample order to all customers
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        for (int j = 0; j < NUM_PRODUCTS; j++) {
            customers[i].order[j] = sample_order[j];
        }
    }

    pthread_t threads[NUM_CUSTOMERS];   //array to hold thread identifiers
    int thread_args[NUM_CUSTOMERS];     //array to hold arguments for each thread

    //initialize the mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Error: unable to initialize mutex\n");
        return 1;
    }

    //create threads for each customer
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        thread_args[i] = i; //argument for the thread is the customer index
        int rc = pthread_create(&threads[i], NULL, customer_thread, &thread_args[i]);   //create the thread
        if (rc) {   //error
            printf("Error: unable to create thread, %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }

    //wait for all threads to complete
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(threads[i], NULL); //join (wait for) each thread
    }

    pthread_mutex_destroy(&lock);   //destroy the mutex

    //display the final inventory and sales
    printf("\nInventory:\n");
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        printf("Product %d: $%.2lf, %d units\n",
               products[i].productID, products[i].productPrice, products[i].productQuantity);
    }

    printf("\nSales:\n");
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        printf("Customer %d:\n", customers[i].customerID); //print each customer's ID
        bool has_purchases = false;
        for (int j = 0; j < NUM_PRODUCTS; j++) {
            if (customers[i].purchasedArr[j] > 0) {
                printf("- Product %d: %d units, $%.2lf each\n", j+1, customers[i].purchasedArr[j], products[j].productPrice);    //print purchased product details
                has_purchases = true;
            }
        }
        if (!has_purchases) {   //if the customer didn't make any purchases
            printf("No purchases made.\n");
        }
    }


    print_latest_balances();    //print the latest balances for each customer

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("\nThe multithread program took %f milliseconds to execute\n", cpu_time_used*1000);    //print time elapsed

    
    return 0;
}


