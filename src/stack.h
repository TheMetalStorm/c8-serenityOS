//
// Created by arapo on 10.09.2024.
//

#ifndef SERENITYOS_STACK_H
#define SERENITYOS_STACK_H

#include "AK/Error.h"
#include "AK/Types.h"
#include "AK/Vector.h"

class Stack {

public:
    Stack(int max) : max_count(max){};
    ~Stack() = default;

    ErrorOr<uint16_t> pop(){
        if(entries.size() == 0){
            return Error::from_string_literal("Stack is empty!");
        }
        auto toReturn = entries.last();
        entries.remove(entries.size()-1);
        return toReturn;
    };
    ErrorOr<void> push(uint16_t in){
        if(entries.size() >= max_count){
            return Error::from_string_literal("Stack Capacity reached!");
        }
        entries.append(in);
        return {};
    }

private:
    size_t max_count = 0;
    Vector<uint16_t> entries;

};

#endif // SERENITYOS_STACK_H
