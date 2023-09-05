// calculator.c

// A simple calculator program capable of addition, subtraction,
// multiplication and division using +, -, *, /.
// By Anujan Cenan, starting on the 15th of August, 2023.

// Notes and Thoughts

/* 
Minor feature to include: a help command which explains how the 
calculator's 'brain' works. (16th of August,)

Can't multiply by negative numbers (yet). Whenever I see a negative, 
I need to check if the previous thing is a mulitply or a divide. If it is,
then I know that I'm dealing with a negative number (probably). 


What are the cases:
(1) If the thing to the left is a multiply/divide and the thing to the right is 
a number, I can make the number the negative version and free the minus.
(2) If the thing on the left is a bracket and the thing on the right is a number
then I can make the number a negative and free the minus.
(3) If there is a plus and a minus in a sequence, then free the plus. (probably
do this first.)
(4) If there is a minus on the left and a number on the right, see (1).
(17th of August, 2023)

Now I seem to be ablel to multiply negative numbers UNLESS there is NOT a 
number on the left hand side of the negative sign e.g. if i do -2 * 3 then
it throws a fit because it doesn't like that there is no number to the left 
of the negative sign. Therefore, I need to make sure all those kind of negative
signs are accounted for BEORE I do the "check the left and right side of each
operator" check. (2nd of September, 2023)

I think I have mainly fixed up the issues from yesterday but now I'm doing some
more error handling work. It seems that if I have an bracket pair "()", but 
without anything inside the brackets, then I have a "segmentation fault."
(3rd of September, 2023)

I have now error handled when a user enters an empty bracket pair "()". 
Besides testing rigorously, I am fairly happy with what I have at the moment.
I think that the primary way forward would be adding a index feature (2^2) 
and a (square) rooting feature. Possibly a factorial as well. Maybe even 
complex one day. (5th of September, 2023) 
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

#define INVALID_INPUT 0
#define VALID_INPUT 1

#define CHAR_INT_CONVERTER 48
#define STARTING_OPERATOR_SEARCH -10

#define NOT_FOUND -1
#define SYNTAX_ERROR -2

// Enums:

enum operations {
    NO_OPERATOR,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION
};

enum decimal_pos {
    LEFT_SIDE,
    RIGHT_SIDE
};

enum type {
    START,
    DOUBLE,
    OPEN_BRACKET,
    CLOSED_BRACKET,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    END
};

// Structs:

struct integer_node {
    int value;
    int position; 
    enum decimal_pos decimal_status;
    struct integer_node *next;
};

struct expression_node {
    enum type input_type;
    double value;
    int position;
    struct expression_node *next;
};

// Function Prototypes:
// Calculator Prototypes
int command_loop();
struct integer_node *handle_integer(
    char input, struct integer_node *head, int decimal
);
struct expression_node *number_in_list(
    struct integer_node *num_head, struct expression_node *exp_head
);

struct expression_node *handle_addition(
    struct integer_node *head, struct expression_node *exp_head
);
struct expression_node *handle_subtraction(
    struct integer_node *num_head, struct expression_node *exp_head
);
struct expression_node *handle_multiplication(
    struct integer_node *num_head, struct expression_node *exp_head
);
struct expression_node *handle_division(
    struct integer_node *num_head, struct expression_node *exp_head
); 
// void handle_decimal();
// void handle_whitespace();
struct expression_node *open_expression(
    struct integer_node *num_head, struct expression_node *exp_head
);
struct expression_node *close_expression(
    struct integer_node *num_head, struct expression_node *exp_head
);
struct expression_node *end_of_expression(
    struct integer_node *num_head, struct expression_node *exp_head
);

// "Evaluating Total" Prototypes
struct expression_node *evaluating_full_expression(
    struct expression_node *head
);

int bracket_syntax_error(
    struct expression_node *head, struct expression_node *prev_to_open_bracket, 
    int closed_bracket_pos
); 

struct expression_node *bodmas(
    struct expression_node *start_node, int final_position, 
    struct expression_node *head, struct expression_node *final_reduction
);

struct expression_node *conduct_operation(struct expression_node *start);

void result_refining(double result);

// "Linked List" Prototypes
struct integer_node *create_digit_node(
    int value, struct integer_node *head, int decimal
);
struct integer_node *kill_num_list(struct integer_node *head);
struct expression_node *kill_expression_list(struct expression_node *head);

struct expression_node *create_expression_node (
    struct expression_node *head, enum type input_type, double value
);

// Expression Handling

struct expression_node *multiply_negative_right_side(struct expression_node *head);
struct expression_node *removing_plus_minus_seq(struct expression_node *head);
struct expression_node *removing_double_operator_seq(struct expression_node *head);
struct expression_node *starting_with_plus_or_minus(struct expression_node *head);

struct expression_node *finding_open_bracket(
    struct expression_node *head, int *closed_bracket_pos
);

struct expression_node *finding_operator(
    struct expression_node *expression_start, 
    int *sign_pos, int end_position, 
    int operator_1, int operator_2
);


// Program Begins 

int main(void) {
    printf("Enter mathematical expression\n");

    while (command_loop() == VALID_INPUT) {
        printf("Enter mathematical expression\n");
    }

    return 0;
}

int command_loop() {
    char input;
    int decimal = FALSE;
    int error_found = 0;
    struct integer_node *head_of_num = NULL;
    struct expression_node *head_of_expression = 
        create_expression_node(NULL, START, 0);

    while (scanf("%c", &input) == 1) {
        if (isdigit(input)) {
            head_of_num = handle_integer(input, head_of_num, decimal);
           } else if (input == '+') {
            head_of_expression = handle_addition(head_of_num, head_of_expression);
            decimal = FALSE;
            head_of_num = kill_num_list(head_of_num);
        } else if (input == '-') {
            head_of_expression = handle_subtraction(head_of_num, head_of_expression);
            decimal = FALSE;
            head_of_num = kill_num_list(head_of_num);
        } else if (input == '*') {
            head_of_expression = handle_multiplication(head_of_num, head_of_expression);
            decimal = FALSE;
            head_of_num = kill_num_list(head_of_num);
        } else if (input == '/') {
            head_of_expression = handle_division(head_of_num, head_of_expression);
            decimal = FALSE;
            head_of_num = kill_num_list(head_of_num);
        } else if (input == '.' && decimal == FALSE) {
            decimal = TRUE;
        } else if (input == ' ') {
            // do nothing            
        } else if (input == '(') {
            decimal = FALSE;
            head_of_expression = open_expression(head_of_num, head_of_expression);
            head_of_num = kill_num_list(head_of_num);
        } else if (input == ')') {
            decimal = FALSE;
            head_of_expression = close_expression(head_of_num, head_of_expression);
            head_of_num = kill_num_list(head_of_num);
        } else if (input == '\n') {
            head_of_expression = end_of_expression(head_of_num, head_of_expression);
            // if user inputs nothing.
            if (head_of_expression->next == NULL) {
                return VALID_INPUT;
            }
            head_of_expression = evaluating_full_expression(head_of_expression);
            head_of_num = kill_num_list(head_of_num);
            head_of_expression = kill_expression_list(head_of_expression);
            return VALID_INPUT;
        } else {
            printf("ERROR: Invalid expression.\n");
            return VALID_INPUT;
        } 
    }
    return INVALID_INPUT;
}

struct integer_node *handle_integer(
    char input, struct integer_node *head, int decimal
) {
    head = create_digit_node(input - CHAR_INT_CONVERTER, head, decimal);
    return head;
}

// Obtains the value of the complete number stored in the number linked list.
// Appends this number to the expression linked list.
struct expression_node *number_in_list(
    struct integer_node *num_head, struct expression_node *exp_head
) {
    struct integer_node *curr_node = num_head;

    if (num_head == NULL) {
        return exp_head;
    } 
    int order_of_magnitude = 0;
    while (curr_node != NULL) {

        if (curr_node->position >= 0) {
            order_of_magnitude = curr_node->position;
        }
        curr_node = curr_node->next;
    }
    curr_node = num_head;
    double number = 0;

    for (int i = order_of_magnitude; i >= 0; i-- ) {
        number += curr_node->value * pow(10, i); 
        curr_node = curr_node->next;
    }

    while (curr_node != NULL) {
        number += curr_node->value * pow(10, curr_node->position);

        curr_node = curr_node->next;
    }

    exp_head = create_expression_node(exp_head, DOUBLE, number);
    return exp_head;
} 

struct expression_node *handle_addition(
    struct integer_node *num_head, struct expression_node *exp_head
) {
    exp_head = number_in_list(num_head, exp_head);
    exp_head = create_expression_node(exp_head, PLUS, 0);
    return exp_head;
}

struct expression_node *handle_subtraction(
    struct integer_node *num_head, struct expression_node *exp_head
) {
    exp_head = number_in_list(num_head, exp_head);
    exp_head = create_expression_node(exp_head, MINUS, 0);
    return exp_head;
}

struct expression_node *handle_multiplication(
    struct integer_node *num_head, struct expression_node *exp_head
) {
    exp_head = number_in_list(num_head, exp_head);
    exp_head = create_expression_node(exp_head, STAR, 0);
    return exp_head;
}

struct expression_node *handle_division(
    struct integer_node *num_head, struct expression_node *exp_head
) {
    exp_head = number_in_list(num_head, exp_head);
    exp_head = create_expression_node(exp_head, SLASH, 0);
    return exp_head;
}

struct expression_node *open_expression(
    struct integer_node *num_head, struct expression_node *exp_head
) {
    exp_head = number_in_list(num_head, exp_head);
    exp_head = create_expression_node(exp_head, OPEN_BRACKET, 0);
    return exp_head;
}

struct expression_node *close_expression(
    struct integer_node *num_head, struct expression_node *exp_head
) {
    exp_head = number_in_list(num_head, exp_head);
    exp_head = create_expression_node(exp_head, CLOSED_BRACKET, 0);
    return exp_head;
}

struct expression_node *end_of_expression(
    struct integer_node *num_head, struct expression_node *exp_head
) {

    exp_head = number_in_list(num_head, exp_head);
    exp_head = create_expression_node(exp_head, END, 0);
    return exp_head;;
} 

struct expression_node *evaluating_full_expression(
    struct expression_node *head
) {

    head = removing_plus_minus_seq(head);
    head = removing_double_operator_seq(head);
    head = multiply_negative_right_side(head);
    head = starting_with_plus_or_minus(head);
    
    int closed_bracket_pos = NOT_FOUND;
    struct expression_node *prev_to_open_bracket = 
        finding_open_bracket(head, &closed_bracket_pos);

    while (prev_to_open_bracket->next != NULL && closed_bracket_pos != -1) {

        struct expression_node *final_reduction = prev_to_open_bracket->next->next;

        // If user enters "()"; that is an empty bracket pair
        if (final_reduction->input_type == CLOSED_BRACKET) {
            printf("You seem to have typed in an empty bracket pair ().\n");
            struct expression_node *open_bracket_delete = 
                prev_to_open_bracket->next;
            struct expression_node *closed_bracket_delete = 
                final_reduction;
            prev_to_open_bracket->next = final_reduction->next;
            free(open_bracket_delete);
            free(closed_bracket_delete);
        } else {
            head = bodmas(
                    prev_to_open_bracket->next, closed_bracket_pos, head, 
                    final_reduction
                );
            struct expression_node *open_bracket_delete = 
                prev_to_open_bracket->next;
            struct expression_node *closed_bracket_delete = 
                final_reduction->next;

            prev_to_open_bracket->next = final_reduction;
            final_reduction->next = closed_bracket_delete->next;
            free(open_bracket_delete);
            free(closed_bracket_delete);
        }
        
        // re-call finding a new bracket pair ()
        prev_to_open_bracket = 
            finding_open_bracket(head, &closed_bracket_pos);
    }

    if (bracket_syntax_error(head, prev_to_open_bracket, closed_bracket_pos)) {
        return NULL;
    }

    struct expression_node *end_node = head;
    while (end_node->next != NULL) {
        end_node = end_node->next;
    }
    struct expression_node *result = head;
    while (result->input_type != DOUBLE) {
        result = result->next;
    }

    head = bodmas(head, end_node->position, head, result);
    if (head == NULL) {
        return NULL;
    }
    result_refining(result->value);
    return head;
}

int bracket_syntax_error(
    struct expression_node *head, struct expression_node *prev_to_open_bracket, 
    int closed_bracket_pos
) {
    // EH: no open bracket, but there is a closing bracket – invalid.    
    if (prev_to_open_bracket->next == NULL && closed_bracket_pos != -1) {
        printf("Invalid input provided, extra )?\n");
        kill_expression_list(head);
        return 1;
    // EH: open bracket provided, but there is no closing bracket – invalid    
    } else if (prev_to_open_bracket->next != NULL && closed_bracket_pos == -1) {
        printf("Invalid input provided, extra (?\n");
        kill_expression_list(head);
        return 1;
    }

    return 0;
}

struct expression_node *bodmas(
    struct expression_node *start_node, int final_position, 
    struct expression_node *head, struct expression_node *final_reduction
) {
    int sign_pos = STARTING_OPERATOR_SEARCH;
    while (sign_pos != NOT_FOUND) {
        
        struct expression_node *start_of_mult_div = 
            finding_operator(
                start_node, &sign_pos, final_position, STAR, SLASH
            );
        
        if (sign_pos == SYNTAX_ERROR) {
            head = kill_expression_list(head);
            return head;
        } else if (sign_pos != NOT_FOUND) {
            start_of_mult_div = conduct_operation(start_of_mult_div);
            final_reduction = start_of_mult_div; 
        }
    }
    sign_pos = STARTING_OPERATOR_SEARCH;
    while (sign_pos != NOT_FOUND) {
        struct expression_node *start_of_add_sub = 
            finding_operator(
                start_node, &sign_pos, final_position, PLUS, MINUS
            );
        
        if (sign_pos == SYNTAX_ERROR) {
            head = kill_expression_list(head);
            return head;
        } else if (sign_pos != NOT_FOUND) {
            start_of_add_sub = conduct_operation(start_of_add_sub);
            final_reduction = start_of_add_sub;
        }
    }
    return head; 
}

struct expression_node *conduct_operation(struct expression_node *start) {

    if (start->next->input_type == STAR) {
        start->value = start->value * start->next->next->value;
    } else if (start->next->input_type == SLASH) {
        start->value = start->value / start->next->next->value;
    } else if (start->next->input_type == PLUS) {
        start->value = start->value + start->next->next->value;
    } else if (start->next->input_type == MINUS) {
        start->value = start->value - start->next->next->value;
    }

    struct expression_node *to_delete1 = start->next;
    struct expression_node *to_delete2 = start->next->next;
    start->next = to_delete2->next;
    free(to_delete1);
    free(to_delete2);

    return start;
}

struct integer_node *create_digit_node(
    int value, struct integer_node *head, int decimal
) {
    
    struct integer_node *node_to_insert = malloc(sizeof(struct integer_node));
    node_to_insert->value = value;
    node_to_insert->next = NULL;
    if (decimal == FALSE) {
        node_to_insert->decimal_status = LEFT_SIDE;
        node_to_insert->position = 0;
    } else {
        node_to_insert->decimal_status = RIGHT_SIDE;
        node_to_insert->position = -1;
    }

    if (head == NULL) {
        head = node_to_insert;
        return head;
    } else {
        struct integer_node *curr_node = head;
        int curr_pos = 0;
        while (curr_node->next != NULL) {
            curr_node = curr_node->next;
            curr_pos++;
        }

        if (curr_node->position >= 0 
            && node_to_insert->decimal_status == LEFT_SIDE) {
            node_to_insert->position = curr_pos + 1;
        } else if (curr_node->position >= 0 
            && node_to_insert->decimal_status == RIGHT_SIDE) {
            node_to_insert->position = -1;
        } else if (curr_node->position < 0) {
            node_to_insert->position = curr_node->position - 1;
        }
        
        curr_node->next = node_to_insert; 
        return head;
    }
}

struct integer_node *kill_num_list(struct integer_node *head) {
    struct integer_node *prev_node = head;
    while (prev_node != NULL) {
        struct integer_node *to_delete = prev_node;
        prev_node = prev_node->next;
        free(to_delete);
    }

    return NULL;
} 

struct expression_node *kill_expression_list(struct expression_node *head) {
    struct expression_node *prev_node = head;
    while (prev_node != NULL) {
        struct expression_node *to_delete = prev_node;
        prev_node = prev_node->next;
        free(to_delete);
    }

    return NULL;
} 

struct expression_node *create_expression_node(
    struct expression_node *head, enum type input_type, double value
) {
    struct expression_node *new_node = malloc(sizeof(struct expression_node));
    new_node->input_type = input_type;
    new_node->value = value;
    new_node->position = 0;
    new_node->next = NULL;

    if (head == NULL) {
        head = new_node;
        return head;
    } else {
        struct expression_node *curr_node = head;
        while (curr_node->next != NULL) {
            curr_node = curr_node->next;
        }

        curr_node->next = new_node;
        new_node->position = curr_node->position + 1;

        return head;
    }
}

struct expression_node *multiply_negative_right_side(struct expression_node *head) {
    struct expression_node *curr_node = head;

    while (curr_node->next->next != NULL) {
        if ((curr_node->input_type == STAR || curr_node->input_type == SLASH)
            && curr_node->next->input_type == MINUS
            && curr_node->next->next->input_type == DOUBLE) {
            
            struct expression_node *to_delete = curr_node->next;
            curr_node->next->next->value *= -1;
            curr_node->next = curr_node->next->next;
            free(to_delete);
        } else {
            curr_node = curr_node->next;
        }
    }
    return head;
}

struct expression_node *removing_plus_minus_seq(struct expression_node *head) {

    struct expression_node *curr_node = head;
    struct expression_node *prev_node = head;
    while (curr_node->next != NULL) {
        // If you have a - + sequence
        if (curr_node->input_type == MINUS 
            && curr_node->next->input_type == PLUS) {
            struct expression_node *to_delete = curr_node->next;
            curr_node->next = curr_node->next->next;
            free(to_delete);
            // curr_node = curr_node->next;
        // If you have a + - sequence
        } else if (curr_node->input_type == PLUS 
            && curr_node->next->input_type == MINUS) {
            curr_node->input_type = MINUS;
            curr_node->next->input_type = PLUS;
        } else {
            prev_node = curr_node;
            curr_node = curr_node->next;
        }
    }
    return head;
} 

struct expression_node *removing_double_operator_seq(struct expression_node *head) {
    struct expression_node *curr_node = head;
    
    while (curr_node->next != NULL) {
        // two pluses
        if (curr_node->input_type == PLUS 
            && curr_node->next->input_type == PLUS) {
            struct expression_node *to_delete = curr_node->next;
            curr_node->next = to_delete->next;
            free(to_delete);
        // double negative
        } else if (curr_node->input_type == MINUS 
            && curr_node->next->input_type == MINUS) {
            struct expression_node *to_delete = curr_node->next;
            curr_node->input_type = PLUS;
            curr_node->next = to_delete->next;
            free(to_delete);
        } 
        else {
            curr_node = curr_node->next;
        }
    }
    return head;
}

struct expression_node *starting_with_plus_or_minus(struct expression_node *head) {
    struct expression_node *curr_node = head;
    while (curr_node->next->next != NULL) {
        if (curr_node->input_type != DOUBLE 
            && curr_node->next->input_type == MINUS
            && curr_node->next->next->input_type == DOUBLE) {
            
            struct expression_node *to_delete = curr_node->next;
            to_delete->next->value *= -1;
            curr_node->next = to_delete->next;
            free(to_delete);    
        } else if (curr_node->input_type != DOUBLE 
            && curr_node->next->input_type == PLUS
            && curr_node->next->next->input_type == DOUBLE) {
            
            struct expression_node *to_delete = curr_node->next;
            curr_node->next = to_delete->next;
            free(to_delete);    
        }

        curr_node = curr_node->next;
    }

    return head;
}

/*
    Returns:
        NULL - no open brackets in the provided expression.
*/
struct expression_node *finding_open_bracket(
    struct expression_node *head, int *closed_bracket_pos
) { 
    *closed_bracket_pos = NOT_FOUND;
    struct expression_node *curr_node = head;

    while (curr_node->next != NULL) {
        curr_node = curr_node->next;
    }
    struct expression_node *prev_to_open_bracket = curr_node;

    curr_node = head;
    while (curr_node->next != NULL) {
        if (curr_node->next->input_type == OPEN_BRACKET) {
            prev_to_open_bracket = curr_node;
        } else if (curr_node->input_type == CLOSED_BRACKET) {
            *closed_bracket_pos = curr_node->position;
            return prev_to_open_bracket;
        }
        curr_node = curr_node->next;
    }

    return prev_to_open_bracket;
}

struct expression_node *finding_operator(
    struct expression_node *expression_start, 
    int *sign_pos, int end_position, 
    int operator_1, int operator_2
) {  
    struct expression_node *curr_node = expression_start->next;  
    struct expression_node *prev_node = expression_start;

    while (curr_node->position != end_position) {

        if (curr_node->input_type == operator_1 
            || curr_node->input_type == operator_2) {
            // If you do not have a number after an operator - BAD CODE
            if (curr_node->next->input_type != DOUBLE) {
                printf("Missing a number? No number after an operator.\n");
                *sign_pos = SYNTAX_ERROR;
                return expression_start;
            } 
            *sign_pos = curr_node->position;
            if (prev_node->input_type != DOUBLE) {
                printf("Missing a number? No number before an operator.\n");
                *sign_pos = SYNTAX_ERROR;
                return expression_start;
            }
            return prev_node;
        }
        prev_node = curr_node;
        curr_node = curr_node->next;
    }
    *sign_pos = NOT_FOUND;
    return expression_start;
}

void result_refining(double result) {
    int result_int = result;
    if (result_int == result) {
        printf("Answer: %d\n", result_int);
        return;
    }

    printf("Answer: %.5lf\n", result);  
}







