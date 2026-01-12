cmake_minimum_required(VERSION 3.28)
project(Arcraven_UGV_Core)

set(CMAKE_CXX_STANDARD 20)

add_executable(Arcraven_UGV_Core
        models/enums/CommandDomain.hpp
        models/enums/UgvCommand.hpp
        models/enums/CommandPriority.hpp
        models/enums/CommandAuthority.hpp
        models/enums/CommandStatus.hpp
        models/enums/RejectReason.hpp

        utils/Logger.hpp
        utils/Logger.cpp
        utils/Base64.hpp
        utils/Base64.cpp

        main.cpp

        core/StateStore.cpp
        config/UgvCore.hpp
        command/CommandRouter.cpp
        subsystems/Iceoryx2Bridge.cpp
)
