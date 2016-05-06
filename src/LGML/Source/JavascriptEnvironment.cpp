/*
 ==============================================================================

 JavascriptEnvironnement.cpp
 Created: 5 May 2016 9:03:35am
 Author:  Martin Hermant

 ==============================================================================
 */

#include "JavascriptEnvironment.h"
#include "TimeManager.h"

juce_ImplementSingleton(JavascriptEnvironment);

JavascriptEnvironment::JavascriptEnvironment(){
    localEnvironment = new DynamicObject();
    localEnvironment->setMethod("post", JavascriptEnvironment::post);
    registerNativeObject("g", localEnvironment);
    linkToControllableContainer("time",TimeManager::getInstance());


}

JavascriptEnvironment::~JavascriptEnvironment(){
    for(auto & n:linkedContainerNamespaces){
        if(n->container.get()) n->container->removeControllableContainerListener(this);
    }
}

void JavascriptEnvironment::linkToControllableContainer(const String & jsNamespace,ControllableContainer * c){
    c->addControllableContainerListener(this);
    DynamicObject::Ptr obj = createDynamicObjectFromContainer(c,nullptr);
    registerNativeObject(jsNamespace, obj);
    if(!existInContainerNamespace(jsNamespace)){
        linkedContainerNamespaces.add(new JsContainerNamespace(jsNamespace,c,obj)) ;

    }}

bool JavascriptEnvironment::existInNamespace(const String & name,const String & module ){
    return localEnvironment->getProperty(name).getDynamicObject()->getProperty(module).getDynamicObject() != nullptr;
}

JavascriptEnvironment::JsContainerNamespace* JavascriptEnvironment::getContainerNamespace(ControllableContainer * c){
    JsContainerNamespace* result = nullptr;
    jassert(c!=nullptr);
    for(auto & n:linkedContainerNamespaces){
        ControllableContainer * inspected = c;
        while(inspected!=nullptr){
            if(n->container == inspected){
                return n;
            }
            inspected = inspected->parentContainer;
        }

    }
    return result;
}
JavascriptEnvironment::JsContainerNamespace* JavascriptEnvironment::getContainerNamespace(const String & n){
    JsContainerNamespace* result = nullptr;
    for(auto & n:linkedContainerNamespaces){
        if(n->nsName==n)
            return n;

    }
    return result;
}

bool JavascriptEnvironment::existInContainerNamespace(const String & n){
    return getContainerNamespace(n)!=nullptr;
}
void    JavascriptEnvironment::removeNamespace(const String & jsNamespace){
    if(JsContainerNamespace * c = getContainerNamespace(jsNamespace)){
        linkedContainerNamespaces.removeObject(c);
        jassertfalse;
    }
    registerNativeObject(jsNamespace, nullptr);
}


DynamicObject* JavascriptEnvironment::createDynamicObjectFromContainer(ControllableContainer * container,DynamicObject *parent){
    DynamicObject*  d = parent;
    if(!container->skipControllableNameInAddress)
        d= new DynamicObject();


    for(auto &c:container->controllables){

        if(Parameter * p = dynamic_cast<Parameter*>(c)){
            DynamicObject* dd= new DynamicObject();

            dd->setProperty("get", p->value);
            dd->setProperty("_ptr", (int64)p);
            dd->setMethod("set", JavascriptEnvironment::set);
            d->setProperty(p->shortName, dd);


        }
        else if(Trigger * t = dynamic_cast<Trigger*>(c)){
            DynamicObject* dd= new DynamicObject();
            dd->setMethod("t", JavascriptEnvironment::set);
            dd->setProperty("_ptr", (int64)t);
            d->setProperty(t->shortName, dd);

        }
    }
    for(auto &c:container->controllableContainers){
        DynamicObject * childObject = createDynamicObjectFromContainer(c,d);
        if(!c->skipControllableNameInAddress)
            d->setProperty(c->shortName, childObject);


    }

    return d;
}


void JavascriptEnvironment::loadFile(const String & path){

    File f (path);
    if(f.existsAsFile() && f.getFileExtension() == ".js"){
        loadedFiles.addIfNotAlreadyThere(path);
        internalLoadFile(f);
    }

}

void JavascriptEnvironment::rebuildAllNamespaces(){
    for(auto & path:loadedFiles){
        File f (path);
        if(f.existsAsFile() && f.getFileExtension() == ".js"){
            internalLoadFile(f);
        }
    }
}

void JavascriptEnvironment::internalLoadFile(const File &f ){
    StringArray destLines;
    f.readLines(destLines);
    String jsString = destLines.joinIntoString("");

    Result r=execute(jsString);
    if(r.failed()){
        DBG("========Javascript error =================");
        DBG(r.getErrorMessage());
    }
}

void JavascriptEnvironment::post(const String & s){
    DBG(s);
}

var JavascriptEnvironment::post(const NativeFunctionArgs& a){
    DBG("posting : "+a.thisObject.toString());
    for(int i = 0 ; i < a.numArguments ;i++){
        post(a.arguments[i].toString());
    }
    return var();

}

void JavascriptEnvironment::addToNamespace(const String & name,const String & elemName,DynamicObject *d){

    DynamicObject * dd = localEnvironment->getProperty(name).getDynamicObject();
    if(dd==nullptr)
    {
        dd= new DynamicObject();
        localEnvironment->setProperty(name, dd);
        registerNativeObject(name, dd);
    }

    dd->setProperty(elemName,d);
}

void JavascriptEnvironment::removeFromNamespace(const String & name,const String & elemName){
    DynamicObject * dd = localEnvironment->getProperty(name).getDynamicObject();
    if(dd!=nullptr) dd->removeProperty(elemName);
}

var JavascriptEnvironment::set(const NativeFunctionArgs& a){

    DynamicObject * d = a.thisObject.getDynamicObject();
    Controllable * c = dynamic_cast<Controllable*>((Controllable*)(int64)d->getProperty("_ptr"));
    bool success = false;

    if (c != nullptr)
    {

        success = true;

        if(a.numArguments==0 && c->type==Controllable::Type::TRIGGER){
            ((Trigger *)c)->trigger();
        }

        else{
            var value = a.arguments[0];
            switch (c->type)
            {
                case Controllable::Type::TRIGGER:
                    if (value.isBool() && (bool)value)
                        ((Trigger *)c)->trigger();
                    else if((value.isDouble()||value.isInt() || value.isInt64())&& (float)value>0)
                        ((Trigger *)c)->trigger();

                    break;

                case Controllable::Type::BOOL:
                    if ( value.isBool())
                        ((BoolParameter *)c)->setValue((bool)value);
                    break;

                case Controllable::Type::FLOAT:
                    if(value.isDouble()||value.isInt() || value.isInt64())
                        ((FloatParameter *)c)->setValue(value);
                    break;
                case Controllable::Type::INT:
                    if(value.isInt() || value.isInt64())
                        ((IntParameter *)c)->setValue(value);
                    break;


                case Controllable::Type::STRING:
                    if(value.isString())
                        ((StringParameter *)c)->setValue(value);
                    break;

                default:
                    success = false;
                    break;

            }
        }
    }
    
    
    return var();
}


void JavascriptEnvironment::childStructureChanged(ControllableContainer * c){
    if(JsContainerNamespace * ns= getContainerNamespace(c)){
        registerNativeObject(ns->nsName, createDynamicObjectFromContainer(ns->container, nullptr));
    }
}



