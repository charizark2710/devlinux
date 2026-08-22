#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct
{
    int id;
    char name[64];
    int age;
    float gpa;
} Student;

Student *readAllStudent(int *c, int *cap)
{
    printf("Retrieve all students \n");
    int capacity = *cap;
    Student student = {0};
    Student *students = malloc(sizeof(Student) * capacity);
    if (!students)
        return NULL;
    int count = *c;

    int fd = open("students.bin", O_RDWR, 0644);
    if (fd == -1)
    {
        perror("Error opening students.bin");
        free(students);
        return NULL;
    }
    while (read(fd, &student, sizeof(Student)) == sizeof(Student))
    {
        if (count >= capacity)
        {
            capacity *= 2;
            Student *temp = realloc(students, capacity * sizeof(Student));
            if (!temp)
            {
                close(fd);
                free(students);
                printf("Error when realloc");
                return NULL;
            }
            students = temp;
        }

        students[count] = student;
        count++;
    }
    close(fd);

    *c = count;
    *cap = capacity;

    return students;
}

int main()
{
    int count = 0;
    int capacity = 2;
    Student *students = readAllStudent(&count, &capacity);
    if (students == NULL)
    {
        return 1;
    }

    int isExit = 0;

    int fd = open("students.bin", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        perror("Error opening students.bin");
        free(students);
        return 1;
    }
    while (isExit == 0)
    {
        int command;
        printf("Menu: \n 1: Add \n 2: List \n 3: Find\n 4: Exit \n Command: ");
        if (scanf("%d", &command) != 1)
        {
            fprintf(stderr, "Error: Invalid input\n");
            close(fd);
            free(students);
            return 1;
        }

        switch (command)
        {
        case 1:
            Student s = {.id = count};

            if (count >= capacity)
            {
                printf("capacity: %d", capacity);
                capacity *= 2;
                Student *temp = realloc(students, capacity * sizeof(Student));
                if (!temp)
                {
                    free(students);
                    close(fd);
                    printf("Error when realloc");
                    return 1;
                }
                students = temp;
            }

            printf("Enter Name: ");
            if (scanf(" %63[^\n]", s.name) != 1)
            {
                fprintf(stderr, "Error: Failed to read name\n");
                close(fd);
                free(students);
                return 1;
            }

            printf("Enter Age: ");
            if (scanf("%d", &s.age) != 1)
            {
                fprintf(stderr, "Error: Failed to read age\n");
                close(fd);
                free(students);
                return 1;
            }

            printf("Enter GPA: ");
            if (scanf("%f", &s.gpa) != 1)
            {
                fprintf(stderr, "Error: Failed to read gpa\n");
                close(fd);
                free(students);
                return 1;
            }

            if (write(fd, &s, sizeof(Student)) != sizeof(Student))
            {
                perror("write failed");
                close(fd);
                free(students);
                return 1;
            }
            students[count] = s;

            count++;

            break;
        case 2:
            for (int i = 0; i < count; i++)
            {
                Student s = students[i];
                printf("ID: %d, Name: %s | Age: %d | GPA: %.2f\n",
                       s.id, s.name, s.age, s.gpa);
            }
            break;
        case 3:
            int id = -1;
            printf("Enter Id: ");
            if (scanf("%d", &id) != 1)
            {
                fprintf(stderr, "Error: Invalid ID\n");
                close(fd);
                free(students);
                return 1;
            }

            int found = 0;
            for (int i = 0; i < count; i++)
            {
                Student s = students[i];
                if (id == s.id)
                {
                    printf("ID: %d, Name: %s | Age: %d | GPA: %.2f\n",
                           s.id, s.name, s.age, s.gpa);
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                printf("not found\n");
            }
            break;
        case 4:
            isExit = 1;
            close(fd);
            break;
        default:
            printf("Command not found \n");
            break;
        }
    }

    free(students);
    return 0;
}