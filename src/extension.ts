import * as vscode from 'vscode';
import * as debug from './commands/debug'
import { problemFetcher } from './util/problemFetcher';
import { stubFileHelperFactory } from './util/stubFileHelper';

export function activate(context: vscode.ExtensionContext) {
	try {
		stubFileHelperFactory.initialize(context);
		context.subscriptions.push(
			problemFetcher,
			vscode.commands.registerCommand("leetcode-cpp-debugger.debug", (uri?: vscode.Uri) => debug.debugSolution(uri))
		);
	}
	catch (error) {
		vscode.window.showInformationMessage(`LeetCode Cpp Debugger initialization error: ${error}`);
	}
}

export function deactivate() {}
