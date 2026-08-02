# Vibe Coding Documentation Standards

> **AI Context**: Tool-neutral Markdown structure standards for canonical project guides and workflow artifacts, designed for reliable parsing by AI agents and human contributors.

## 1. Structure for Active Guides
Every active guide in `docs/` MUST adhere to the following format:

1. **Header & Context (Top Section)**
   - Must always start with an H1 (`# Clear Document Title`).
   - The second line must always be a Blockquote defining the context for AI.
   ```markdown
   # Document Title
   
   > **AI Context**: [A brief, precise 1-2 sentence description of the file's purpose so AI can parse it quickly].
   ```

2. **Alerts & Constraints**
   - It is mandatory to use GitHub Flavored Markdown (GFM) Alerts for strict rules. LLMs parse these blocks highly efficiently.
   - `> [!WARNING]`: Used for rules that MUST NOT be violated (e.g., No JS in QML).
   - `> [!IMPORTANT]`: Used for critical architectural decisions.
   - `> [!NOTE]`: Used for general observations or contextual notes.

3. **Code Formatting**
   - Every opening code fence MUST declare its language explicitly.
   - Use tags such as `cpp`, `qml`, `cmake`, `bash`, `text`, or `mermaid`. Never leave an opening fence untagged.

## 2. Canonical Documentation Hygiene

Markdown in this repository records the current useful project truth, not every abandoned path.

- Keep final behavior, active constraints, accepted rationale, open risks, and current verification evidence.
- Remove or collapse discarded prototypes, reverted implementation paths, temporary visual drafts, and plans that returned to the original baseline.
- Use Git history for detailed archaeology. Do not keep stale Markdown only to prove that a path was attempted.
- Dated specifications, plans, and reports may use their workflow metadata instead of `> **AI Context**:`, but they must still represent active or durable guidance. Delete abandoned artifacts instead of labeling them as old directions.

## 3. Troubleshooting Section
Every active technical guide (for example `hardware_integration.md`, `architecture.md`, `testing_strategy.md`, and `ui_ux_guidelines.md`) MUST include a **Troubleshooting** section at the bottom.
- **Purpose**: When the AI (Vibe Coder) encounters a build error or logic bug, it can autonomously look up this section to self-correct without asking the human for help.

## 4. Memory & Workflows System
- **`tasks_board.md`**: Should only contain task lists (Todo, In-Progress, Done). No lengthy explanations.
- **`journal.md`**: The place to document accepted technical decisions and rationale that still help future work. It is not a complete attempt log.
- **`.agents/workflows/`**: The designated location for step-by-step workflow guidelines (e.g., TDD, Brainstorming) instead of cluttering the System Prompt.
