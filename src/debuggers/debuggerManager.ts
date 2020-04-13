import { Debugger } from "./debugger"
import { CppDebugger } from "./cppDebugger"
import { StubFileHelper } from "../util/stubFileHelper";

export interface IDebuggerConstructor {
    new(codeTemplate: string, stubFileHelper: StubFileHelper): Debugger
}

export function getDebugger(language: string): IDebuggerConstructor | undefined {
    switch (language) {
        case "cpp": return CppDebugger;
    }
    // unsupported yet!
    return undefined;
}
