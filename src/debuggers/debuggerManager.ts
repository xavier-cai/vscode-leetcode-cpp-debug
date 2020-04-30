import { Debugger } from "./debugger"
import { CppDebugger } from "./cppDebugger"
import { StubFileHelper } from "../util/stubFileHelper";
import { StubCodeHelper } from "../util/stubCodeHelper";

export interface IDebuggerConstructor {
    new(codeTemplate: string, stubCodeHelper: StubCodeHelper, stubFileHelper: StubFileHelper): Debugger
}

export function getDebugger(language: string): IDebuggerConstructor | undefined {
    switch (language) {
        case "cpp": return CppDebugger;
    }
    // unsupported yet!
    return undefined;
}
