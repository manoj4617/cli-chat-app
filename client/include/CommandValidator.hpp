#ifndef COMMANDVALIDATOR_HPP
#define COMMANDVALIDATOR_HPP

#include <optional>
#include <string>
#include "AppState.hpp"
#include "ClientCommands.hpp"

using ValidatorError = std::optional<std::string>;

class CommandValidator {
    public:
        ValidatorError validate(const ClientCommand&, AppState&);
    private:
        ValidatorError validate(const LoginCommand&, AppState&);
        ValidatorError validate(const LogoutCommand&, AppState&);
        ValidatorError validate(const JoinBarrackCommand&, AppState&);
        ValidatorError validate(const LeaveBarrackCommand&, AppState&);
        ValidatorError validate(const SendMessageCommand&, AppState&);
        ValidatorError validate(const GetBarrackCommand&, AppState&);
        ValidatorError validate(const CreateBarrackCommand&, AppState&);
        ValidatorError validate(const DestroyBarrackCommand&, AppState&);
        ValidatorError validate(const CreateUserCommand&, AppState&);
        
        template<typename T>
        ValidatorError validate(const T&, AppState &state){
            return std::nullopt;
        }

};

#endif