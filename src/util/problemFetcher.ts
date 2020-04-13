import * as request from 'request';
import {Source, getSource, Language} from './config'
import * as vscode from 'vscode';
import * as uc from './config';

interface IProblem {
    id: string;
    fid: string;
    name: string;
    slug: string;
}

class ProblemFetcher implements vscode.Disposable {
    private cacheSource: Source;
    private cacheProblems: IProblem[];
    private cacheId: string | undefined;
    private cacheContent: any;
    private configurationChangeListener: vscode.Disposable;
    private readonly leetCodeApiProblems: string = "api/problems/algorithms";
    private readonly leetCodeApiGraphql: string = "graphql";

    public constructor() {
        this.cacheSource = Source.Local;
        this.cacheProblems = [];
        this.configurationChangeListener = vscode.workspace.onDidChangeConfiguration((event: vscode.ConfigurationChangeEvent) => {
            if (event.affectsConfiguration("leetcode-cpp-debugger.source")) {
                //this.updateProblems();
            }
        }, this);
        //this.updateProblems();
    }

    public dispose() {
        this.configurationChangeListener.dispose();
    }

    private async request(rq: any, options: any): Promise<any> {
        return new Promise<any>((resolve, reject) => {
            rq(options, (e: any, resp: any, body: any) => {
                if (e || !resp || resp.statusCode != 200) {
                    reject(`Request connect error.`);
                    return;
                }
                resolve(body);
            });
        });
    }

    private async updateProblems() {
        const source: Source | undefined = getSource();
        if (!source) {
            throw new Error("Invalid source.");
        }

        if (source == Source.Local || source == this.cacheSource) {
            return;
        }

        // init
        this.cacheSource = Source.Local;

        const body: any = await this.request(request.get, {
            uri: uc.getHost() + this.leetCodeApiProblems,
            headers: {},
            timeout: uc.configs.timeout
        });

        const json = JSON.parse(body);
        this.cacheProblems = (json.stat_status_pairs as any[])
            .filter(p => !p.stat.question__hide)
            .map(function(p): IProblem {
                return {
                    id:       p.stat.question_id,
                    fid:      p.stat.frontend_question_id,
                    name:     p.stat.question__title,
                    slug:     p.stat.question__title_slug
                };
            });
        this.cacheSource = source;
    }
    
    public async fetchProblem(id: string, language: Language) : Promise<string> {
        await this.updateProblems();
        const problem: IProblem | undefined = this.cacheProblems.find((v: IProblem) => v.fid == id);
        if (!problem) {
            throw new Error("Invalid problem ID.");
        }
        if (!this.cacheId || this.cacheId != id) {
            const body: any = await this.request(request.post, {
                uri: uc.getHost() + this.leetCodeApiGraphql,
                headers: {},
                timeout: uc.configs.timeout,
                json: true,
                body: {
                    query: [
                        'query getQuestionDetail($titleSlug: String!) {',
                        '  question(titleSlug: $titleSlug) {',
                        '    codeDefinition',
                        '    sampleTestCase',
                        '  }',
                        '}'
                    ].join('\n'),
                    variables: {titleSlug: problem.slug},
                    operationName: 'getQuestionDetail'
                }
            });
            const q = body.data.question;
            if (!q) {
                throw new Error("Invalid problem ID.");
            }
            this.cacheId = id;
            this.cacheContent = {
                codeDefinition: JSON.parse(q.codeDefinition),
                sampleTestCase: q.sampleTestCase
            };
        }
        const code: any | undefined = (this.cacheContent.codeDefinition as any[]).find((p) => p.value == language.name);
        if (!code || !code.defaultCode) {
            throw new Error(`Cant find code definition with language: ${language.disp}.`);
        }
        return code.defaultCode;
    }
}

export const problemFetcher: ProblemFetcher = new ProblemFetcher();