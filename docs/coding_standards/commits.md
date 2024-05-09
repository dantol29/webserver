**Effective commit messages**

Writing effective commit messages is crucial for maintaining a clear and accessible project history. A widely respected approach is the one popularized by the Angular team: _Conventional Commits_. This standard helps automate the versioning and changelog generation processes while maintaining readability and consistency in commit logs.

We decided to adopt [Conventional Commits v1.0.0](https://www.conventionalcommits.org/en/v1.0.0/) during the development of our project

**Key Guidelines:**

- **Structured Format:** Each commit message should have a clear structure that includes a type, a scope (optional), and a subject: `<type>(<scope>): <subject>`

- **Types:** Common types include:
- `feat`: new feature
- `fix`: bug fix
- `docs`: documentation changes
- `style`: formatting, missing semicolons, etc.
- `refactor`: code change that neither fixes a bug nor adds a feature
- `test`: adding missing tests
- `chore`: maintenance tasks, e.g., updating grunt tasks

- **Scope:** The scope can specify the area of the commit change (e.g., module name, file name).

- **Subject:** The subject contains a succinct description of the change:
- Use the imperative mood ("add", not "adds" or "added").
- Do not capitalize the first letter.
- No dot (.) at the end.

- **Body and Footer (optional but recommended for more complex changes):**
- The body should include a detailed explanation of what changed and why, not how.
- The footer can contain information about breaking changes and is also used to reference issue tracker IDs.

**Examples:**

1. **Simple Commit:**  
   `fix(login): correct minor typos in code`
   `fix(api): handle null responses from the server`
   `docs(readme): add installation instructions`
   `test(cart): update tests for cart subtotal calculation`
   `refactor(search): optimize query handling`
   `chore(deps): bump axios version to 0.24.0`

2. **Complex Commit:**  
   `feat(blog): add comment section`
   > Users will now be able to post comments on blog posts.
   >
   > **BREAKING CHANGE:** Database schema updated. Run migrations before deploying.  
   > Resolves #123

These guidelines help keep the project's commit history informative and navigable, especially useful in collaborative environments where multiple developers need to understand each other's changes quickly and clearly. Other models and modifications exist, such as using emojis or more specific types, but the _Conventional Commits_ standard is a good starting point for most projects.
