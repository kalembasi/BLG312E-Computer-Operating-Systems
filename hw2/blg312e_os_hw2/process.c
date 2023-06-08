//omer malik kalembasi  
//150180112

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define NUM_CUSTOMERS 3 
#define NUM_PRODUCTS 5

//customer struct
typedef struct {
    int customerID;
    double customerBalance;
    int orderedArr[NUM_PRODUCTS];
    int purchasedArr[NUM_PRODUCTS];
} Customer;

//product struct
typedef struct {
    int productID;
    double productPrice;
    int productQuantity;
} Product;

Customer *customers; //pointer to shared memory for customers
Product *products; //pointer to shared memory for products


//initialize products with random prices and quantities
void initialize_products() {
    double productPrice, productQuantity;
    double price, quantity;
    srand(time(NULL));
    for (int idx = 0; idx < NUM_PRODUCTS; idx++) {
        price = (double)rand()/(double)(RAND_MAX/200);  //generate product price in the range of 1 – 200$.
        quantity = (double)rand()/(double)(RAND_MAX/10);  //generate product quantity in the range of 1-10 for each.
        products[idx].productID = idx+1;
        products[idx].productPrice = price;
        products[idx].productQuantity = quantity;
    }
}

//initialize customers with random balances and empty order and purchase lists
void initialize_customers() {
    double balance;
    srand(time(NULL));
    for (int idx = 0; idx < NUM_CUSTOMERS; idx++) {
        balance = (double)rand()/(double)(RAND_MAX/200); // Generate customer's available balance in the range of 0 – 200$
        customers[idx].customerID = idx+1;
        customers[idx].customerBalance = balance;
        for (int jdx = 0; jdx < NUM_PRODUCTS; jdx++) {
            customers[idx].orderedArr[jdx] = 0;
            customers[idx].purchasedArr[jdx] = 0;
        }
    }
}

//order a product for a customer
void order_product(Customer *customer, Product *product, int quantity) {
    if (product->productQuantity >= quantity) {
        double balance = customer->customerBalance;
        if (balance >= product->productPrice*quantity) {
            product->productQuantity -= quantity;
            customer->customerBalance -= product->productPrice*quantity;
            customer->orderedArr[product->productID-1] += quantity;
            customer->purchasedArr[product->productID-1] += quantity; //update purchasedArr array
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
}

//order products for a customer
void order_products(Customer *customer, int items[NUM_PRODUCTS]) {
    printf("Customer %d has ordered:\n", customer->customerID);
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        if (items[i] > 0) {
            order_product(customer, &products[i], items[i]);    //order each product
        }
    }
}

//print initial balances of all customers
void print_initial_balances() {
    printf("Initial customer balances:\n");
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        printf("Customer %d: $%.2lf\n", customers[i].customerID, customers[i].customerBalance);
    }
    printf("\n");
}

//print initial inventory of all products
void print_initial_inventory() {
    printf("\nInitial inventory:\n");
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        printf("Product %d: $%.2lf, %d units\n",
               products[i].productID, products[i].productPrice, products[i].productQuantity);
    }
    printf("\n");
}

//print the latest balances of all customers
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

    //create shared memory segments for customers and products
    int shmid_customers = shmget(IPC_PRIVATE, sizeof(Customer) * NUM_CUSTOMERS, IPC_CREAT | 0666);
    int shmid_products = shmget(IPC_PRIVATE, sizeof(Product) * NUM_PRODUCTS, IPC_CREAT | 0666);

    //attach shared memory segments to process address space
    customers = (Customer *)shmat(shmid_customers, NULL, 0);
    products = (Product *)shmat(shmid_products, NULL, 0);

    if (customers == (Customer *)-1 || products == (Product *)-1) {
        printf("Error: shmat failed\n");    //print error if shmat fails
        exit(EXIT_FAILURE);
    }

    initialize_products();  //initialize the products
    initialize_customers(); //initialize the customers

    print_initial_inventory(); 
    print_initial_balances();  

    int items[NUM_PRODUCTS] = {2, 1, 3, 1, 1};  //define the items quantity to be ordered by each customer

    pid_t pid;
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pid = fork();   //create a child process for each customer
        if (pid == 0) {
            order_products(&customers[i], items);   //child process orders products
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        wait(NULL); //parent process waits for all child processes to finish
    }

    //print remaining inventory
    printf("\nInventory:\n");
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        printf("Product %d: $%.2lf, %d units\n",
               products[i].productID, products[i].productPrice, products[i].productQuantity);
    }

    //print sales made by each customer
    printf("\nSales:\n");
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        printf("Customer %d:\n", customers[i].customerID);
        bool has_purchases = false;
        for (int j = 0; j < NUM_PRODUCTS; j++) {
            if (customers[i].purchasedArr[j] > 0) {
                printf("- Product %d: %d units, $%.2lf each\n",
                       j+1, customers[i].purchasedArr[j], products[j].productPrice);
                has_purchases = true;
            }
        }
        if (!has_purchases) {
            printf("No purchases made.\n");
        }
    }

    print_latest_balances();    //print latest balances

    //detach and free shared memory segments
    shmdt(customers);
    shmdt(products);
    shmctl(shmid_customers, IPC_RMID, NULL);
    shmctl(shmid_products, IPC_RMID, NULL);

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("\nThe multiprocess program took %f milliseconds to execute\n", cpu_time_used*1000);    //print time elapsed

    return 0;
}
