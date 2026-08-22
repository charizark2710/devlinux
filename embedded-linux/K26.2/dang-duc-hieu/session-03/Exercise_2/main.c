#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct
{
    int id;
    char name[64];
    int quantity;
    double price;
} Product;

int getProductCount(int fd)
{
    off_t fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize == -1)
        return 0;
    return (int)(fileSize / sizeof(Product));
}

int main(void)
{
    int fd = open("products.dat", O_RDWR | O_CREAT, 0644);
    if (fd == -1)
    {
        perror("Error opening products.dat");
        return 1;
    }

    int isExit = 0;

    while (isExit == 0)
    {
        int command;
        printf("\nMenu:\n 1: Add product\n 2: Show product by index\n 3: Update quantity by index\n 4: List all products\n 5: Exit\n Command: ");

        if (scanf("%d", &command) != 1)
            break;

        switch (command)
        {
        case 1:
        {
            Product p = {0};
            int totalRecords = getProductCount(fd);
            p.id = totalRecords;

            printf("Enter Name: ");
            if (scanf(" %63[^\n]", p.name) != 1)
            {
                fprintf(stderr, "Error: Failed to read name\n");
                break;
            }

            printf("Enter Quantity: ");
            if (scanf("%d", &p.quantity) != 1)
            {
                fprintf(stderr, "Error: Failed to read quantity\n");
                break;
            }

            printf("Enter Price: ");
            if (scanf("%lf", &p.price) != 1)
            {
                fprintf(stderr, "Error: Failed to read price\n");
                break;
            }
            lseek(fd, 0, SEEK_END);

            if (write(fd, &p, sizeof(Product)) != sizeof(Product))
            {
                perror("write failed");
                break;
            }
            printf("Product added successfully with ID/Index: %d\n", p.id);
            break;
        }
        case 2:
        {
            int index;
            printf("Enter Index: ");
            if (scanf("%d", &index) != 1)
            {
                fprintf(stderr, "Error: Invalid index\n");
                break;
            }
            int totalRecords = getProductCount(fd);
            if (index < 0 || index >= totalRecords)
            {
                printf("Error: Index out of bounds (Total products: %d)\n", totalRecords);
                break;
            }

            lseek(fd, index * sizeof(Product), SEEK_SET);

            Product p;
            if (read(fd, &p, sizeof(Product)) == sizeof(Product))
            {
                printf("ID: %d | Name: %s | Quantity: %d | Price: $%.2f\n",
                       p.id, p.name, p.quantity, p.price);
            }
            break;
        }
        case 3:
        {
            int index, newQty;
            printf("Enter Index: ");
            if (scanf("%d", &index) != 1)
            {
                fprintf(stderr, "Error: Invalid index\n");
                break;
            }

            int totalRecords = getProductCount(fd);
            if (index < 0 || index >= totalRecords)
            {
                printf("Error: Index out of bounds (Total products: %d)\n", totalRecords);
                break;
            }

            printf("Enter New Quantity: ");
            if (scanf("%d", &newQty) != 1)
            {
                fprintf(stderr, "Error: Invalid index input\n");
                break;
            }
            off_t offset = index * sizeof(Product);
            lseek(fd, offset, SEEK_SET);

            Product p;
            if (read(fd, &p, sizeof(Product)) == sizeof(Product))
            {
                p.quantity = newQty;

                lseek(fd, offset, SEEK_SET);
                if (write(fd, &p, sizeof(Product)) != sizeof(Product))
                {
                    perror("Error updating product");
                    break;
                }
                printf("Quantity updated successfully!\n");
            }
            break;
        }
        case 4:
        {
            int totalRecords = getProductCount(fd);
            if (totalRecords == 0)
            {
                printf("No products found.\n");
                break;
            }

            lseek(fd, 0, SEEK_SET);
            Product p;
            int idx = 0;

            while (read(fd, &p, sizeof(Product)) == sizeof(Product))
            {
                printf("[%d] ID: %d | Name: %s | Quantity: %d | Price: $%.2f\n",
                       idx++, p.id, p.name, p.quantity, p.price);
            }
            break;
        }
        case 5:
            isExit = 1;
            break;
        default:
            printf("Command not found\n");
            break;
        }
    }

    close(fd);
    return 0;
}