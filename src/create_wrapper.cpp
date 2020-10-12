#include "tree.hpp"
#include "create_wrapper.hpp"
#include "globals.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdio.h>

void
WrapperCreator::create_wrapper(Namespace* ns)
{
    std::string fromfile = original_file != "" ? original_file : inputfile;

    if(selected_namespace != "") {
        ns_prefix = selected_namespace;
        ns_prefix += "::";
    }

    // hpp file
    hppout
        << "/**\n"
        << " * WARNING: This file is automatically generated from:\n"
        << " *  '" << fromfile << "'\n"
        << " * DO NOT CHANGE\n"
        << " */\n"
        << "#ifndef HEADER_SUPERTUX_SCRIPTING_WRAPPER_HPP\n" //TODO avoid hardcoding
        << "#define HEADER_SUPERTUX_SCRIPTING_WRAPPER_HPP\n"
        << "\n"
        << "#include <squirrel.h>\n"
        << "\n"
        << "namespace Scripting {\n"
        << "\n";

    hppout << "void register_" << modulename << "_wrapper(HSQUIRRELVM v);\n"
           << "\n";

    for(auto& type : ns->types) {
        auto* _class = dynamic_cast<Class*> (type);
        if(_class == nullptr)
            continue;

        hppout << "class " << _class->name << ";\n";
        hppout << "void create_squirrel_instance(HSQUIRRELVM v, "
               << ns_prefix << _class->name
               << "* object, bool setup_releasehook = false);\n";
    }
    hppout <<"\n"
           << "}\n"
           << "\n"
           << "#endif\n"
           << "\n"
           << "/* EOF */\n";

    // cpp header
    out << "/**\n"
        << " * WARNING: This file is automatically generated from:\n"
        << " *  '" << fromfile << "'\n"
        << " * DO NOT CHANGE\n"
        << " */\n"
        << "\n"
        << "#include \"scripting/wrapper.hpp\"\n"
        << "\n"
        << "#include <assert.h>\n"
        << "#include <limits>\n"
        << "#include <sstream>\n"
        << "#include <squirrel.h>\n"
        << "\n"
        << "#include \"scripting/squirrel_error.hpp\"\n"
        << "#include \"scripting/wrapper.interface.hpp\"\n"
        << "\n"
        << "namespace Scripting {\n"
        << "namespace Wrapper {\n"
        << "\n";

    for(auto& type : ns->types) {
        auto* _class = dynamic_cast<Class*> (type);
        if(_class != nullptr)
            create_class_wrapper(_class);
    }
    for(auto& func : ns->functions) {
        create_function_wrapper(nullptr, func);
    }

    out << "} // namespace Wrapper\n";

    for(auto& type : ns->types) {
        Class* _class = dynamic_cast<Class*> (type);
        if(_class != nullptr)
            create_squirrel_instance(_class);
    }

    out << "void register_" << modulename << "_wrapper(HSQUIRRELVM v)\n"
        << "{\n"
        << ind << "using namespace Wrapper;\n"
        << "\n";

    create_register_constants_code(ns);
    create_register_functions_code(ns);
    create_register_classes_code(ns);

    out << "}\n"
        << "\n"
        << "} // namespace Scripting\n"
        << "\n"
        << "/* EOF */\n";
}

void
WrapperCreator::create_register_function_code(Function* function, Class* _class)
{
    if(function->type == Function::DESTRUCTOR)
        return;

    out << ind << "sq_pushstring(v, \"" << function->name << "\", -1);\n";
    out << ind << "sq_newclosure(v, &"
        << (_class != nullptr ? _class->name + "_" : "") << function->name
        << "_wrapper, 0);\n";

    if(function->custom) {
      out << ind << "sq_setparamscheck(v, SQ_MATCHTYPEMASKSTRING, " << function->parameter_spec << ");\n";
    } else {
      out << ind << "sq_setparamscheck(v, SQ_MATCHTYPEMASKSTRING, \"";

      out << "x|t";

      if(!function->parameters.empty())
        {
          std::vector<Parameter>::iterator p = function->parameters.begin();

          // Skip the first parameter since its a HSQUIRRELVM that is
          // handled internally
          if (function->suspend) {
            ++p;
          } else if (p->type.atomic_type == HSQUIRRELVMType::instance()) {
            ++p;
          }

          for(; p != function->parameters.end(); ++p) {
            if(p->type.atomic_type == &BasicType::INT) {
              out << "i";
            } else if(p->type.atomic_type == &BasicType::FLOAT) {
              out << "n";
            } else if(p->type.atomic_type == &BasicType::BOOL) {
              out << "b";
            } else if(p->type.atomic_type == StringType::instance()) {
              out << "s";
            } else {
              out << ".";
            }
          }
      }
      out << "\");\n";
    }

    create_register_slot_code("function", function->name);
    out << "\n";
}

void
WrapperCreator::create_register_functions_code(Namespace* ns)
{
    for(auto& function : ns->functions) {
        create_register_function_code(function, nullptr);
    }
}

void
WrapperCreator::create_register_classes_code(Namespace* ns)
{
    for(auto& type : ns->types) {
        auto* _class = dynamic_cast<Class*> (type);
        if(_class == nullptr)
            continue;
        if(_class->super_classes.size() > 0)
            continue;

        create_register_class_code(_class);
    }
}

void
WrapperCreator::create_register_class_code(Class* _class)
{
    out << ind << "// Register class " << _class->name << "\n";
    out << ind << "sq_pushstring(v, \""
        << _class->name << "\", -1);\n";

    if(_class->super_classes.size() > 0) {
        if(_class->super_classes.size() > 1) {
            std::ostringstream msg;
            msg << "Multiple inheritance not supported (at class '"
                << _class->name << "')";
            throw std::runtime_error(msg.str());
        }

        out << ind << "sq_pushstring(v, \""
            << _class->super_classes[0]->name << "\", -1);\n";
        out << ind << "sq_get(v, -3);\n";
    }
    out << ind << "if(sq_newclass(v, "
        << (_class->super_classes.size() > 0 ? "SQTrue" : "SQFalse")
        << ") < 0) {\n";
    out << ind << ind << "std::ostringstream msg;\n";
    out << ind << ind << "msg << \"Couldn't create new class '"
        << _class->name << "'\";\n";
    out << ind << ind << "throw SquirrelError(v, msg.str());\n";
    out << ind << "}\n";

    for(auto& member : _class->members) {
        if(member->visibility != ClassMember::PUBLIC)
            continue;
        auto* function = dynamic_cast<Function*> (member);
        if(function) {
            create_register_function_code(function, _class);
        }
        auto* field = dynamic_cast<Field*> (member);
        if(field) {
            create_register_constant_code(field);
        }
    }

    create_register_slot_code("class", _class->name);
    out << "\n";

    for(auto& c : _class->sub_classes) {
        create_register_class_code(c);
    }
}

void
WrapperCreator::create_register_constants_code(Namespace* ns)
{
    for(auto& field: ns->fields) {
        create_register_constant_code(field);
    }
}

void
WrapperCreator::create_register_constant_code(Field* field)
{
    if(!field->has_const_value)
        return;
    out << ind << "sq_pushstring(v, \"" << field->name << "\", -1);\n";
    if(field->type->atomic_type == &BasicType::INT) {
        out << ind << "sq_pushinteger(v, " << field->const_int_value << ");\n";
    } else if(field->type->atomic_type == &BasicType::FLOAT) {
        out << ind << "sq_pushfloat(v, " << field->const_float_value << ");\n";
    } else if(field->type->atomic_type == StringType::instance()) {
        out << ind << "sq_pushstring(v, \""
            << field->const_string_value << "\", -1);\n";
    } else {
      throw std::runtime_error("Constant is not int, float or string");
    }
    create_register_slot_code("constant", field->name);
    out << "\n";
}

void
WrapperCreator::create_register_slot_code(const std::string& what,
                                          const std::string& name)
{
    out << ind << "if(SQ_FAILED(sq_createslot(v, -3))) {\n";
    out << ind << ind << "throw SquirrelError(v, \""
        << "Couldn't register " << what << " '" << name << "'\");\n";
    out << ind << "}\n";
}

void
WrapperCreator::create_function_wrapper(Class* _class, Function* function)
{
    if(function->type == Function::DESTRUCTOR)
        assert(false);

    std::string ns_prefix;
    if(selected_namespace != "")
        ns_prefix = selected_namespace + "::";
    if(function->type == Function::CONSTRUCTOR)
        function->name = "constructor";

    out << "static SQInteger ";
    if(_class != nullptr) {
        out << _class->name << "_";
    }
    out << function->name << "_wrapper(HSQUIRRELVM vm)\n"
        << "{\n";
    // avoid warning...
    if(_class == nullptr && function->parameters.empty()
            && function->return_type.is_void()
            && function->type != Function::CONSTRUCTOR) {
        out << ind << "(void) vm;\n";
    }

    // retrieve pointer to class instance
    if(_class != nullptr && function->type != Function::CONSTRUCTOR) {
        out << ind << "SQUserPointer data;\n";
        out << ind << "if(SQ_FAILED(sq_getinstanceup(vm, 1, &data, nullptr)) || !data) {\n";
        out << ind << ind << "sq_throwerror(vm, _SC(\"'" << function->name << "' called without instance\"));\n";
        out << ind << ind << "return SQ_ERROR;\n";
        out << ind << "}\n";
        out << ind << ns_prefix << _class->name << "* _this = reinterpret_cast<" << ns_prefix << _class->name << "*> (data);\n";
        out << "\n";
        out << ind << "if (_this == nullptr) {\n";
        out << ind << ind << "return SQ_ERROR;\n";
        out << ind << "}\n";
        out << "\n";
    }

    // custom function?
    if(function->custom) {
        if(function->type != Function::FUNCTION)
            throw std::runtime_error(
                    "custom not allow constructor+destructor yet");
        if(function->return_type.atomic_type != SQIntegerType::instance())
            throw std::runtime_error("custom function '" + function->name + "' has to return SQInteger");
        if(function->parameters.size() != 1)
            throw std::runtime_error(
                    "custom function '" + function->name + "' must have 1 HSQUIRRELVM parameter");

        out << ind << "return ";
        if(_class != nullptr)
            out << "_this->";
        else
            out << ns_prefix;
        out << function->name << "(vm);\n";
        out << "}\n";
        out << "\n";
        return;
    }

    // declare and retrieve arguments
    int i = 0;
    int arg_offset = 2;
    for(auto& p : function->parameters) {
        if(i == 0 && p.type.atomic_type == HSQUIRRELVMType::instance()) {
            out << ind << "HSQUIRRELVM arg0 = vm;\n";
            arg_offset--;
        } else {
            char argname[64];
            snprintf(argname, sizeof(argname), "arg%d", i);
            prepare_argument(p.type, i + arg_offset, argname);
        }
        ++i;
    }

    // call function
    out << "\n";
    out << ind << "try {\n";
    out << ind << ind;
    if(!function->return_type.is_void()) {
        function->return_type.write_c_type(out);
        out << " return_value = ";
    }
    if(_class != nullptr) {
        if(function->type == Function::CONSTRUCTOR) {
            out << "auto _this = new " << ns_prefix;
        } else {
            out << "_this->";
        }
    } else {
        out << ns_prefix;
    }
    if(function->type == Function::CONSTRUCTOR) {
        assert(_class != nullptr);
        out << _class->name << "(";
    } else {
        out << function->name << "(";
    }
    for(size_t i = 0; i < function->parameters.size(); ++i) {
        if(i != 0)
            out << ", ";
        const Parameter param = function->parameters[i];
        if(param.type.ref == 0 && param.type.pointer == 0) {
            if(param.type.atomic_type == &BasicType::INT)
                out << "static_cast<int> (arg" << i << ")";
            else if(param.type.atomic_type == &BasicType::FLOAT)
                out << "arg" << i << "";
            else if(param.type.atomic_type == &BasicType::BOOL)
                out << "arg" << i << " == SQTrue";
            else
                out << "arg" << i;
        } else {
            out << "arg" << i;
        }
    }
    out << ");\n";
    if(function->type == Function::CONSTRUCTOR) {
        assert(_class != nullptr);
        out << ind << "if(SQ_FAILED(sq_setinstanceup(vm, 1, _this))) {\n";
        out << ind << ind << "sq_throwerror(vm, _SC(\"Couldn't setup instance of '" << _class->name << "' class\"));\n";
        out << ind << ind << "return SQ_ERROR;\n";
        out << ind << "}\n";
        out << ind << "sq_setreleasehook(vm, 1, "
            << _class->name << "_release_hook);\n";
    }
    out << "\n";
    // push return value back on stack and return
    if(function->suspend) {
        if(!function->return_type.is_void()) {
            std::stringstream msg;
            msg << "Function '" << function->name << "' declared as suspend"
                << " but has a return value.";
            throw std::runtime_error(msg.str());
        }
        out << ind << ind << "return sq_suspendvm(vm);\n";
    } else if(function->return_type.is_void()) {
        out << ind << ind << "return 0;\n";
    } else {
        push_to_stack(function->return_type, "return_value");
        out << ind << ind << "return 1;\n";
    }

    out << "\n";
    out << ind << "} catch(std::exception& e) {\n";
    out << ind << ind << "sq_throwerror(vm, e.what());\n";
    out << ind << ind << "return SQ_ERROR;\n";
    out << ind << "} catch(...) {\n";
    out << ind << ind << "sq_throwerror(vm, _SC(\"Unexpected exception while executing function '" << function->name << "'\"));\n";
    out << ind << ind << "return SQ_ERROR;\n";
    out << ind << "}\n";
    out << "\n";

    out << "}\n";
    out << "\n";
}

void
WrapperCreator::prepare_argument(const Type& type, size_t index,
        const std::string& var)
{
    if(type.ref > 0 && type.atomic_type != StringType::instance())
        throw std::runtime_error("References not handled yet");
    if(type.pointer > 0)
        throw std::runtime_error("Pointers not handled yet");
    if(type.atomic_type == &BasicType::INT) {
        out << ind << "SQInteger " << var << ";\n";
        out << ind << "if(SQ_FAILED(sq_getinteger(vm, " << index << ", &" << var << "))) {\n";
        out << ind << ind << "sq_throwerror(vm, _SC(\"Argument " << (index-1) << " not an integer\"));\n";
        out << ind << ind << "return SQ_ERROR;\n";
        out << ind << "}\n";
    } else if(type.atomic_type == &BasicType::FLOAT) {
        out << ind << "SQFloat " << var << ";\n";
        out << ind << "if(SQ_FAILED(sq_getfloat(vm, " << index << ", &" << var << "))) {\n";
        out << ind << ind << "sq_throwerror(vm, _SC(\"Argument " << (index-1) << " not a float\"));\n";
        out << ind << ind << "return SQ_ERROR;\n";
        out << ind << "}\n";
    } else if(type.atomic_type == &BasicType::BOOL) {
        out << ind << "SQBool " << var << ";\n";
        out << ind << "if(SQ_FAILED(sq_getbool(vm, " << index << ", &" << var << "))) {\n";
        out << ind << ind << "sq_throwerror(vm, _SC(\"Argument " << (index-1) << " not a bool\"));\n";
        out << ind << ind << "return SQ_ERROR;\n";
        out << ind << "}\n";
    } else if(type.atomic_type == StringType::instance()) {
        out << ind << "const SQChar* " << var << ";\n";
        out << ind << "if(SQ_FAILED(sq_getstring(vm, " << index << ", &" << var << "))) {\n";
        out << ind << ind << "sq_throwerror(vm, _SC(\"Argument " << (index-1) << " not a string\"));\n";
        out << ind << ind << "return SQ_ERROR;\n";
        out << ind << "}\n";
    } else {
        std::ostringstream msg;
        msg << "Type '" << type.atomic_type->name << "' not supported yet.";
        throw std::runtime_error(msg.str());
    }
}

void
WrapperCreator::push_to_stack(const Type& type, const std::string& var)
{
    if(type.ref > 0 && type.atomic_type != StringType::instance())
        throw std::runtime_error("References not handled yet");
    if(type.pointer > 0)
        throw std::runtime_error("Pointers not handled yet");
    out << ind << ind;
    if(type.atomic_type == &BasicType::INT) {
        out << "sq_pushinteger(vm, " << var << ");\n";
    } else if(type.atomic_type == &BasicType::FLOAT) {
        out << "sq_pushfloat(vm, " << var << ");\n";
    } else if(type.atomic_type == &BasicType::BOOL) {
        out << "sq_pushbool(vm, " << var << ");\n";
    } else if(type.atomic_type == StringType::instance()) {
        out << "assert(" << var << ".size() < static_cast<size_t>(std::numeric_limits<SQInteger>::max()));\n"
            << ind << ind << "sq_pushstring(vm, " << var << ".c_str(), static_cast<SQInteger>("
            << var << ".size()));\n";
    } else {
        std::ostringstream msg;
        msg << "Type '" << type.atomic_type->name << "' not supported yet.";
        throw std::runtime_error(msg.str());
    }
}

void
WrapperCreator::create_class_wrapper(Class* _class)
{
    create_class_release_hook(_class);
    for(auto& member : _class->members) {
        if(member->visibility != ClassMember::PUBLIC)
            continue;
        Function* function = dynamic_cast<Function*> (member);
        if(!function)
            continue;
        // don't wrap destructors
        if(function->type == Function::DESTRUCTOR)
            continue;
        create_function_wrapper(_class, function);
    }
}

void
WrapperCreator::create_squirrel_instance(Class* _class)
{
    out << "void create_squirrel_instance(HSQUIRRELVM v, "
        << ns_prefix << _class->name
        << "* object, bool setup_releasehook)\n"
        << "{\n"
        << ind << "using namespace Wrapper;\n"
        << "\n"
        << ind << "sq_pushroottable(v);\n"
        << ind << "sq_pushstring(v, \"" << _class->name << "\", -1);\n"
        << ind << "if(SQ_FAILED(sq_get(v, -2))) {\n"
        << ind << ind << "std::ostringstream msg;\n"
        << ind << ind << "msg << \"Couldn't resolved squirrel type '"
        << _class->name << "'\";\n"
        << ind << ind << "throw SquirrelError(v, msg.str());\n"
        << ind << "}\n"
        << "\n"
        << ind << "if(SQ_FAILED(sq_createinstance(v, -1)) || "
        << "SQ_FAILED(sq_setinstanceup(v, -1, object))) {\n"
        << ind << ind << "std::ostringstream msg;\n"
        << ind << ind << "msg << \"Couldn't setup squirrel instance for "
        << "object of type '" << _class->name << "'\";\n"
        << ind << ind << "throw SquirrelError(v, msg.str());\n"
        << ind << "}\n"
        << ind << "sq_remove(v, -2); // remove object name\n"
        << "\n"
        << ind << "if(setup_releasehook) {\n"
        << ind << ind << "sq_setreleasehook(v, -1, "
        << _class->name << "_release_hook);\n"
        << ind << "}\n"
        << "\n"
        << ind << "sq_remove(v, -2); // remove root table\n"
        << "}\n"
        << "\n";
}

void
WrapperCreator::create_class_release_hook(Class* _class)
{
    out << "static SQInteger " << _class->name << "_release_hook(SQUserPointer ptr, SQInteger )\n"
        << "{\n"
        << ind << ns_prefix << _class->name << "* _this = reinterpret_cast<" << ns_prefix << _class->name
        << "*> (ptr);\n"
        << ind << "delete _this;\n"
        << ind << "return 0;\n"
        << "}\n"
        << "\n";
}
